/*
 * Start tcpip server
 */

// This is a Matlab mex function.
#include "mex.h"

#include <iostream>
#include <map>
#include <deque>
#include <vector>
#include <stdexcept>

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

// This map is used to store data passed over from matlab space in the
// case that the connection for the named data has not yet been
// created. Suppose you passed in an array of 1000 numbers for the
// connection called "PopulationA", but the SpineML experiment had not
// yet connected that connection. The data will be placed into this
// map as a "holding position". When the connection is then created,
// any data here is then moved into the connection object. This data
// structure is available to other matlab mex functions in the
// spinemlnet_ family, as its pointer address is passed into matlab
// space.
//
// Note how we define DATACACHE_MAP_DEFINED. Other code which
// #includes SpineMLConnection.h has leave this undefined and a dummy
// dataCache pointer will be instantiated at global scope.
//
#define DATACACHE_MAP_DEFINED 1
map<string, deque<double>*>* dataCache;
pthread_mutex_t dataCacheMutex;

// Include our connection class code.
#include "SpineMLConnection.h"

// Allow up to 1024 bytes in the listen queue.
#define LISTENQ 1024

// thread handle and termination flag, must be global. This is the
// main server thread. Each incoming connection then gets its own
// thread in addition to this one.
pthread_t thread;
// flags
volatile bool stopRequested;  // User requested stop from matlab space
volatile bool clientStopRequested;  // Used by main thread to request stop on client threads.
volatile bool threadFinished; // The main thread finished executing (maybe it failed), need to inform
                              // matlab space
volatile bool initialised;    // Set when server is up and running - this only refers to the main
                              // thread, which polls for new connections.

// This map is indexed by the thread id of the connection. Available
// to matlab mex functions as its pointer address is passed into
// matlab space.
map<pthread_t, SpineMLConnection*>* connections;

/*
 * Close the network sockets. Uses a global for connecting_socket;
 * listening socket is passed in.
 */
void closeSockets (int& listening_socket)
{
    cout << "SpineMLNet: start-closeSockets: Called" << endl;

    // Close each connection:
    map<pthread_t, SpineMLConnection*>::iterator connectionsIter = connections->begin();
    while (connectionsIter != connections->end()) {
        connectionsIter->second->closeSocket();
        ++connectionsIter;
    }

    if (close (listening_socket)) {
        int theError = errno;
        cout << "SpineMLNet: start-closeSockets: Error closing listening socket: " << theError << endl;
    }
    cout << "SpineMLNet: start-closeSockets: Returning" << endl;
}

/*!
 * Close only the main thread's listening socket.
 */
void closeSocket (int& listening_socket)
{
    cout << "SpineMLNet: start-closeSocket: Called" << endl;

    if (close (listening_socket)) {
        int theError = errno;
        cout << "SpineMLNet: start-closeSocket: Error closing listening socket: " << theError << endl;
    }
    cout << "SpineMLNet: start-closeSocket: Returning" << endl;
}

/*!
 * Initialise the server with socket(), bind(), listen(). Return
 * listening socket.
 */
int initServer (void)
{
    // Set up and await connection from a TCP/IP client. Use
    // the socket(), bind(), listen() and accept() calls.
    cout << "SpineMLNet: start-initServer: Open a socket." << endl;
    int listening_socket = socket (AF_INET, SOCK_STREAM, 0);
    if (listening_socket < 0) {
        // error.
        cout << "SpineMLNet: start-initServer: Failed to open listening socket." << endl;
        return -1;
    }

    // This is the port on which this server will listen.
    int port = 50091;

    struct sockaddr_in servaddr;
    bzero((char *) &servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    int bind_rtn = bind (listening_socket, (struct sockaddr *) &servaddr, sizeof(servaddr));
    if (bind_rtn < 0) {
        int theError = errno;
        cout << "SpineMLNet: start-initServer: Failed to bind listening socket (error " << theError << ")." << endl;
        return -1;
    }

    int listen_rtn = listen (listening_socket, LISTENQ);
    if (listen_rtn < 0) {
        cout << "SpineMLNet: start-initServer: Failed to listen to listening socket." << endl;
        return -1;
    }

    return listening_socket;
}

/*!
 * Code executed for each accepted connection.
 */
void* connectionThread (void*)
{
    cout << "SpineMLNet: start-connectionThread: New thread starting." << endl;

    /*
     * First job is to get connected - that is to complete the handshake.
     */
    pthread_t myThread = pthread_self(); // Figure out own thread ID to get correct connection.
    SpineMLConnection* c;

    // spin until connection got added to connections. See ***
    bool setup = false;
    while (!setup) {
        try {
            c = (*connections)[myThread];
            setup = true;
        } catch (const exception& e) {
            cout << "SpineMLNet: start-connectionThread: No connection for myThread ("
                 << (long unsigned int*)myThread << ") yet..." << endl;
        }
        usleep (1000);
    }

    cout << "SpineMLNet: start-connectionThread: start while loop..." << endl;
    while (!stopRequested /*&& !c->getFailed()*/) {
        // Is the connection established?
        //cout << "SpineMLNet: start-connectionThread: Loop.." << endl;
        if (!c->getEstablished()) {
            //cout << "SpineMLNet: start-connectionThread: Not established. Do handshake." << endl;
            if (c->doHandshake() < 0) {
                cout << "SpineMLNet: start-pollForConnection: Failed to complete SpineML handshake." << endl;
                // Now what kind of clean up?
                c->closeSocket();
                break;
            }
            cout << "SpineMLNet: start-pollForConnection: Completed handshake." << endl;
        } else {
            //cout << "SpineMLNet: start-connectionThread: Established. Do data I/O." << endl;
            /*
             * Now do data I/O
             */
            int retval = c->doInputOutput();
            if (retval == -1) {
                // Read or write to that connection failed. Do we now set stopRequested true?
                cout << "SpineMLNet: start-connectionThread: doInputOutput failed for thread 0x"
                     << hex << (int)myThread << dec << endl;
                c->closeSocket();
                break;
            } else if (retval == 1) {
                // Connection finished; client disconnected.
                cout << "SpineMLNet: start-connectionThread: Connection finished." << endl;
                c->closeSocket();
                break;
            }
        }
    }

    return NULL;
}

/*!
 * Poll for a connection on listening socket. If one is found, do the
 * handshake and create an entry in the connections map.
 */
int pollForConnection (int& listening_socket, struct pollfd& p)
{
    // cout << "SpineMLNet: start-pollForConnection: called" << endl;
    int retval = 0;
    p.revents = 0;
    if ((retval = poll (&p, 1, 0)) > 0) {
        // This is ok.
        //cout << "Got positive value from poll()"<< endl;
    } else if (retval == -1) {
        int theError = errno;
        cout << "SpineMLNet: start-pollForConnection: error with poll(), errno: "
             << theError << endl;
        return -1;
    } else {
        // This is ok.
        //cout << "poll returns 0." << endl;
    }

    if (p.revents & POLLIN || p.revents & POLLPRI) {
        // Data ready to read...
        int connecting_socket = accept (listening_socket, NULL, NULL);
        if (connecting_socket < 0) {
            int theError = errno;
            cout << "SpineMLNet: start-pollForConnection: Failed to accept on listening socket. errno: "
                 << theError << endl;
            return -1;
        } // else connected ok.

        // Create a new connection instance and insert it into our map container
        SpineMLConnection* c = new SpineMLConnection();
        c->setConnectingSocket (connecting_socket);

        // Create a thread for this connection to operate in
        int rtn = pthread_create (&c->thread, NULL, &connectionThread, NULL);
        if (rtn < 0) {
            cout << "SpineMLNet: start-pollForConnection: Failed to create a connection thread." << endl;
            return -1;
        }
        connections->insert (make_pair (c->thread, c)); // *** connectionThread needs to wait until this
                                                        // insert completes before getting on with its
                                                        // business.

        cout << "SpineMLNet: start-pollForConnection: Accepted a connection." << endl;

    } // else no new connections available on the listening socket

    return 0;
}

void cleanupFailedConnections (void)
{
    map<pthread_t, SpineMLConnection*>::iterator connectionsIter = connections->begin();

    while (connectionsIter != connections->end()) {
        if (connectionsIter->second->getFailed() == true) {
            cout << "SpineMLNet: start-cleanupFailedConnections: we have a failed connection. allow to join." << endl;
            pthread_join (connectionsIter->first, 0);
            cout << "SpineMLNet: start-cleanupFailedConnections: now delete the object." << endl;
            delete connectionsIter->second;
            cout << "SpineMLNet: start-cleanupFailedConnections: now remove pointer from map." << endl;
            connections->erase (connectionsIter);
        }
        ++connectionsIter;
    }
}

/*
 * Delete connections.
 */
void deleteConnections (void)
{
    cout << "SpineMLNet: start-deleteConnections: called" << endl;

    map<pthread_t, SpineMLConnection*>::iterator connectionsIter = connections->begin();

    // First job - run through and destroy all threads.
    cout << "SpineMLNet: start-deleteConnections: join threads" << endl;
    while (connectionsIter != connections->end()) {
        cout << "SpineMLNet: start-deleteConnections: join a thread..." << endl;
        pthread_join (connectionsIter->first, 0);
        ++connectionsIter;
    }

    // Now delete the connections
    cout << "SpineMLNet: start-deleteConnections: delete connections" << endl;
    connectionsIter = connections->begin();
    while (connectionsIter != connections->end()) {
        cout << "SpineMLNet: start-deleteConnections: delete a connection" << endl;
        delete connectionsIter->second;
        ++connectionsIter;
    }

    // Clear, then delete the connections map.
    cout << "SpineMLNet: start-deleteConnections: clear and deallocate connections map" << endl;
    connections->clear();
    delete connections;
}

/*
 * thread function - this is where the TCP/IP comms happens. This is a
 * matlab-free zone.
 */
void* theThread (void* nothing)
{
    // Initialisation
    threadFinished = false;
    connections = new map<pthread_t, SpineMLConnection*>();
    int listening_socket = initServer();
    if (listening_socket == -1) {
        threadFinished = true;
        return NULL;
    }
    // We poll for activity on the connection, so that if the user
    // Ctrl-Cs we don't block on an accept() call.
    struct pollfd p;
    p.fd = listening_socket;
    p.events = POLLIN|POLLPRI;
    p.revents = 0;
    int retval = 0;

    // The main thread is now initialised.
    initialised = true;

    // loop until we get the termination signal
    while (!stopRequested) {

        /*
         * First job in the loop is to see if we have any more
         * connections coming in from the client.
         */
        retval = pollForConnection (listening_socket, p);
        if (retval != 0) {
            threadFinished = true;
            return NULL;
        }

        /*
         * Second job in loop is to do some housekeeping.
         */
        cleanupFailedConnections();

        usleep (10000);

    } // while (!stopRequested)

    // Shutdown code
    cout << "SpineMLNet: start-theThread: Close sockets." << endl;

    // Clean up.
    deleteConnections();
    closeSocket (listening_socket);

    threadFinished = true; // Here, its "threadFinished" really

    cout << "SpineMLNet: start-theThread: At end of thread." << endl;
    return NULL;
}

void mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    // initialise flags
    stopRequested = false;
    initialised = false;
    threadFinished = false;

    // Allocate the dataCache memory
    dataCache = new map<string, deque<double>*>();
    pthread_mutex_init (&dataCacheMutex, NULL);

    // create the thread
    cout << "SpineMLNet: start-mexFunction: creating thread..." << endl;
    int rtn = pthread_create (&thread, NULL, &theThread, NULL);
    if (rtn < 0) {
        cout << "SpineMLNet: start-mexFunction: Failed to create thread." << endl;
        return;
    }

    // wait until initialisation in the thread is complete before continuing
    do {
        usleep (1000);
        if (threadFinished == true) {
            // Shutdown as we have an error.
            cout << "SpineMLNet: start-mexFunction: Shutdown due to error." << endl;

            // for each connection, delete it (thereby also destroying the mutexes):
            deleteConnections();

            // clear the loop
            initialised = true;
        }
    } while (!initialised);
    cout << "SpineMLNet: start-mexFunction: Main thread is initialised." << endl;

    // details of output
    mwSize dims[2] = { 1, 16 };
    plhs[0] = mxCreateNumericArray (2, dims, mxUINT64_CLASS, mxREAL);
    // pointer to pass back the context to matlab
    unsigned long long int* context = (unsigned long long int*) mxGetData (plhs[0]);

    // store thread information - this passed back to matlab as context is plhs[0].
    context[0] = (unsigned long long int)&thread;
    context[1] = (unsigned long long int)&stopRequested;
    context[2] = (unsigned long long int)&threadFinished;
    context[3] = (unsigned long long int)connections;
    context[4] = (unsigned long long int)dataCache;
    context[5] = (unsigned long long int)&dataCacheMutex;
}
