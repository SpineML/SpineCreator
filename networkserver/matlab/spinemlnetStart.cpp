/*
 * Start tcpip server
 */

// This is a Matlab mex function.
#include "mex.h"

#include <iostream>
#include <deque>

extern "C" {
#include <pthread.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <poll.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
}

using namespace std;

// thread handle and termination flag, must be global
pthread_t thread;
// flags
volatile bool stopRequested; // User requested stop from matlab space
volatile bool threadFailed;  // Thread failed, need to inform matlab space
volatile bool updated;       // Data was updated. May or may not need this - comes from mkinect.
volatile char clientDataDirection; // AM_SOURCE or AM_TARGET
volatile char clientDataType;      // nums, spikes or impulses. Only nums implemented.
// mutexes
pthread_mutex_t bufferMutex;
// data structures
volatile char* bufferData;
// Whatever data type is used by SpineML:
volatile std::deque<int> data;

// flag for if we are all up and running
volatile bool initialised;

// A connected socket.
volatile int connecting_socket;

volatile int errorFlag;

// Allow up to 1024 bytes in the listen queue.
#define LISTENQ 1024

// SpineML tcp/ip comms flags.
#define RESP_DATA_NUMS     31 // a non-printable character
#define RESP_DATA_SPIKES   32 // ' ' (space)
#define RESP_DATA_IMPULSES 33 // '!'
#define RESP_HELLO         41 // ')'
#define RESP_RECVD         42 // '*'
#define RESP_ABORT         43 // '+'
#define RESP_FINISHED      44 // ','
#define AM_SOURCE          45 // '-'
#define AM_TARGET          46 // '.'
#define NOT_SET            99 // 'c'

// SpineML tcp/ip comms data types
enum dataTypes {
    ANALOG,
    EVENT,
    IMPULSE
};

#define CS_HS_GETTINGTARGET   0
#define CS_HS_GETTINGDATATYPE 1
#define CS_HS_READINGDUMMY    2
#define CS_HS_DONE            3
int
doHandshake (void)
{
    ssize_t b = 0;
    char buf[16];
    // There are three stages in the handshake process:
    int handshakeStage = CS_HS_GETTINGTARGET;
    while (handshakeStage != CS_HS_DONE) {

        if (handshakeStage == CS_HS_GETTINGTARGET) {
            b = read (connecting_socket, (void*)buf, 1);
            if (b == 1) {
                // Got byte.
                if (buf[0] == AM_SOURCE || buf[0] == AM_TARGET) {
                    clientDataDirection = buf[0];
                    // Write response.
                    buf[0] = RESP_HELLO;
                    if (write (connecting_socket, buf, 1) != 1) {
                        cout << "SpineMLNet: Failed to write RESP_HELLO to client." << endl;
                        return -1;
                    }
                    handshakeStage++;
                } else {
                    // Wrong data direction.
                    clientDataDirection = NOT_SET;
                    cout << "SpineMLNet: Wrong data direction in first handshake byte from client." << endl;
                    return -1;
                }
            } // else b==0, so try reading again.

        } else if (handshakeStage == CS_HS_GETTINGDATATYPE) {
            b = read (connecting_socket, (void*)buf, 1);
            if (b == 1) {
                // Got byte.
                if (buf[0] == RESP_DATA_NUMS || buf[0] == 'a') { // a is for test/debug
                    clientDataType = buf[0];
                    buf[0] = RESP_RECVD;
                    if (write (connecting_socket, buf, 1) != 1) {
                        cout << "SpineMLNet: Failed to write RESP_RECVD to client." << endl;
                        return -1;
                    }
                    handshakeStage++;

                } else if (buf[0] == RESP_DATA_SPIKES || buf[0] == RESP_DATA_IMPULSES) {
                    // These are not yet implemented.
                    cout << "SpineMLNet: Spikes/Impulses not yet implemented." << endl;
                    return -1;
                }
            }

        } else if (handshakeStage == CS_HS_READINGDUMMY) {
            b = read (connecting_socket, (void*)buf, 4);
            if (b == 4) {
                // Got 4 bytes.
                buf[0] = RESP_RECVD;
                if (write (connecting_socket, buf, 1) != 1) {
                    cout << "SpineMLNet: Failed to write RESP_RECVD to client." << endl;
                    return -1;
                }
                handshakeStage++;

            } else {
                // Wrong number of bytes.
                cout << "SpineMLNet: Read " << b << " bytes, expected 4." << endl;
                for (ssize_t i = 0; i<b; ++i) {
                    cout << "buf[" << i << "]: '" << buf[i] << "'  0x" << hex << buf[i] << dec << endl;
                }
                return -1;
            }

        } else if (handshakeStage == CS_HS_DONE) {
            cout << "SpineMLNet: Handshake finished." << endl;
        } else {
            cout << "SpineMLNet: Error: Invalid handshake stage." << endl;
            return -1;
        }
    }
    return 0;
}

// Close the network sockets. Uses a global for connecting_socket;
// listening socket is passed in.
void
closeSockets (int& listening_socket)
{
    cout << "SpineMLNet: Closing sockets." << endl;
    if (close (connecting_socket)) {
        int theError = errno;
        cout << "SpineMLNet: Error closing connecting socket: " << theError << endl;
    }
    if (close (listening_socket)) {
        int theError = errno;
        cout << "SpineMLNet: Error closing listening socket: " << theError << endl;
    }
}

// thread function - this is where the TCP/IP comms happens. This is a
// matlab-free zone.
void*
theThread (void* nothing)
{
    // INIT CODE
    threadFailed = false;
    errorFlag = 0;

    // Set up and await connection from a TCP/IP client. Use
    // the socket(), bind(), listen() and accept() calls.
    int listening_socket = socket (AF_INET, SOCK_STREAM, 0);
    if (listening_socket < 0) {
        // error.
        cout << "SpineMLNet: Failed to open listening socket." << endl;
        threadFailed = true;
        return NULL;
    }

    // This is the port on which this server will listen.
    int port = 50099;

    struct sockaddr_in servaddr;
    bzero((char *) &servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(port);

    int bind_rtn = bind (listening_socket, (struct sockaddr *) &servaddr, sizeof(servaddr));
    if (bind_rtn < 0) {
        int theError = errno;
        cout << "SpineMLNet: Failed to bind listening socket (error " << theError << ")." << endl;
        threadFailed = true;
        return NULL;
    }

    int listen_rtn = listen (listening_socket, LISTENQ);
    if (listen_rtn < 0) {
        cout << "SpineMLNet: Failed to listen to listening socket." << endl;
        threadFailed = true;
        return NULL;
    }

    // MAIN LOOP CODE

    // We poll for activity on the connection, so that if the user
    // Ctrl-Cs we don't block on an accept() call.
    struct pollfd p;
    p.fd = listening_socket;
    p.events = POLLIN|POLLPRI;
    p.revents = 0;
    int retval = 0;

    // loop until we get the termination signal
    bool connected = false;
    while (!stopRequested) {

        if (!connected) {

            p.revents = 0;

            if ((retval = poll (&p, 1, 0)) > 0) {
                // This is ok.
                // cout << "Got positive value from select() or poll()"<< endl;
            } else if (retval == -1) {
                cout << "error with poll()/select()" << endl;
            } else {
                // This is ok.
                // cout << "poll returns 0." << endl;
            }

            if (p.revents & POLLIN || p.revents & POLLPRI) {
                // Data ready to read...
                connecting_socket = accept (listening_socket, NULL, NULL);
                if (connecting_socket < 0) {
                    cout << "SpineMLNet: Failed to accept on listening socket." << endl;
                    threadFailed = true;
                    return NULL;
                }

                connected = true;
                cout << "SpineMLNet: Accepted a connection." << endl;

                // Now we have a connection, we can procede to carry out
                // the SpineML TCP/IP network comms handshake
                //
                if (doHandshake() < 0) {
                    cout << "SpineMLNet: Failed to complete SpineML handshake." << endl;
                    threadFailed = true;
                    closeSockets (listening_socket);
                    return NULL;
                }
                cout << "SpineMLNet: Completed handshake." << endl;

            } else {
                // cout << "no data available." << endl;
            }

        } else { // connected

            ///////// update some data in memory
            pthread_mutex_lock (&bufferMutex);

            // Update the buffer by reading/writing from network.

            pthread_mutex_unlock (&bufferMutex);

            initialised = true;
        }
    }

    // Shutdown code
    cout << "SpineMLNet: Thread shutting down." << endl;
    closeSockets (listening_socket);

    return NULL;
}

void
mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    // initialise flags
    stopRequested = false;
    initialised = false;
    updated = false;
    errorFlag = 1;

    // set up mutexes
    pthread_mutex_init (&bufferMutex, NULL);

    // create the thread
    cout << "SpineMLNet: creating thread...";
    int rtn = pthread_create (&thread, NULL, &theThread, NULL);
    if (rtn < 0) {
        cout << "SpineMLNet: Failed to create thread." << endl;
        return;
    }

    // wait until initialisation in the thread is complete before continuing
    do {
        usleep (1000);
        if (errorFlag !=0) {
            // Shutdown as we have an error.
            cout << "Shutdown due to error." << endl;
            // destroy mutexes
            pthread_mutex_destroy(&bufferMutex);
            // clear the loop
            initialised = true;
        }
    } while (!initialised);

    // details of output
    mwSize dims[2] = { 1, 16 };
    plhs[0] = mxCreateNumericArray (2, dims, mxUINT64_CLASS, mxREAL);
    // pointer to pass back the context to matlab
    unsigned long long int* threadPtr = (unsigned long long int*) mxGetData (plhs[0]);

    // store thread information - this passed back to matlab as threadPtr is plhs[0].
    threadPtr[0] = (unsigned long long int)&thread;
    threadPtr[1] = (unsigned long long int)&stopRequested;
    threadPtr[2] = (unsigned long long int)&updated;
    threadPtr[3] = (unsigned long long int)bufferData;
    threadPtr[4] = (unsigned long long int)&bufferMutex;
    threadPtr[5] = (unsigned long long int)&threadFailed;
}
