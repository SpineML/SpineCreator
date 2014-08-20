/*
 * Start tcpip server
 */

#ifdef COMPILE_OCTFILE
# include <octave/oct.h>
#else
# ifndef char16_t
// To enable compilation on Mac OS X 10.8.
typedef unsigned short char16_t;
# endif
# include "mex.h"
#endif

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

// A mutex to keep our dbg output messages from being garbled.
pthread_mutex_t* coutMutex;

// Include our connection class code.
#include "SpineMLConnection.h"

// Defines the DBG and INFO macros for thread-safe cout.
#include "SpineMLDebug.h"

// Allow up to 1024 bytes in the listen queue.
#define LISTENQ 1024

// The thread handle. This is the main server thread. Each incoming
// connection then gets its own thread in addition to this one. This
// global handle is accessed from other mex functions via its address.
pthread_t thread;

// Server state flags
volatile bool stopRequested;          // User requested stop from matlab space
volatile bool connectionsFinished;    // All running connections completed.
volatile bool threadFinished;         // The main thread finished executing (maybe it failed);
                                      // used to inform matlab space.
volatile bool initialised;            // Set when server is up and running - this only refers
                                      // to the main thread, which polls for new connections.

// This map is indexed by the thread id of the connection. Available
// to matlab mex functions as its pointer address is passed into
// matlab space.
map<pthread_t, SpineMLConnection*>* connections;

// The port on which the server will listen.
#define DEFAULT_PORT 50091
int port;

/*!
 * Close the network sockets. Uses a global for connecting_socket;
 * listening socket is passed in.
 */
void closeSockets (int& listening_socket)
{
    INFO ("start-closeSockets: Closing sockets.");

    // Close each connection:
    map<pthread_t, SpineMLConnection*>::iterator connectionsIter = connections->begin();
    while (connectionsIter != connections->end()) {
        connectionsIter->second->closeSocket();
        ++connectionsIter;
    }

    if (close (listening_socket)) {
        int theError = errno;
        INFO ("start-closeSockets: Error closing listening socket: " << theError);
    }
}

/*!
 * Close only the main thread's listening socket.
 */
void closeSocket (int& listening_socket)
{
    if (close (listening_socket)) {
        int theError = errno;
        INFO ("start-closeSocket: Error closing listening socket "
              << listening_socket << ": " << theError);
    } else {
        INFO ("start-closeSocket: Successfully closed listening socket " << listening_socket);
    }
}

/*!
 * Returns true if any of the established connections are AM_SOURCE;
 * that is data is being transferred from the client to this server.
 */
bool haveAmSourceConnections (void)
{
    bool rtn = false;
    map<pthread_t, SpineMLConnection*>::iterator connectionsIter = connections->begin();
    while (connectionsIter != connections->end()) {
        if (connectionsIter->second->getClientDataDirection() == AM_SOURCE) {
            rtn = true;
            break;
        }
        ++connectionsIter;
    }
    return rtn;
}

/*!
 * Initialise the server with socket(), bind(), listen(). Return
 * listening socket.
 */
int initServer (void)
{
    // Set up and await connection from a TCP/IP client. Use
    // the socket(), bind(), listen() and accept() calls.
    INFO ("start-initServer: Open, bind and listen...");
    int listening_socket = socket (AF_INET, SOCK_STREAM, 0);
    if (listening_socket < 0) {
        // error.
        INFO ("start-initServer: Failed to open listening socket.");
        return -1;
    } else {
        INFO ("start-initServer: Opened listening_socket " << listening_socket);
    }

    struct sockaddr_in servaddr;
    bzero((char *) &servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    int bind_rtn = bind (listening_socket, (struct sockaddr *) &servaddr, sizeof(servaddr));
    if (bind_rtn < 0) {
        int theError = errno;
        INFO ("start-initServer: Failed to bind listening socket (error " << theError << ").");
        return -1;
    } else {
        INFO ("start-initServer: Bound port " << port << " to socket " << listening_socket);
    }

    int listen_rtn = listen (listening_socket, LISTENQ);
    if (listen_rtn < 0) {
        INFO ("start-initServer: Failed to listen to listening socket.");
        return -1;
    }

    return listening_socket;
}

/*!
 * Code executed for each accepted connection.
 */
void* connectionThread (void*)
{
    INFO ("start-connectionThread: New thread starting.");

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
            INFO ("start-connectionThread: No connection for myThread ("
                  << (long unsigned int*)myThread << ") yet...");
        }
        usleep (1000);
    }

    while (!stopRequested) {

        // Is the connection established?
        if (!c->getEstablished()) {

            // Yes, it is established.
            if (c->doHandshake() < 0) {
                INFO ("start-pollForConnection: Failed to complete SpineML handshake.");
                // Close the socket to clean up
                c->closeSocket();
                break;
            }
            INFO ("start-pollForConnection: Completed handshake.");

        } else {

            // Now do data I/O
            int retval = c->doInputOutput();
            if (retval == -1) {
                // Read or write to that connection failed. Do we now set stopRequested true?
                INFO ("start-connectionThread: doInputOutput failed for thread 0x"
                     << hex << (unsigned long long int)myThread << dec);
                c->closeSocket();
                break;
            } else if (retval == 1) {
                // Connection finished; client disconnected.
                INFO ("start-connectionThread: Connection "
                      << c->getClientConnectionName() << " finished.");
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
    int retval = 0;
    p.revents = 0;
    if ((retval = poll (&p, 1, 0)) == -1) {
        int theError = errno;
        INFO ("start-pollForConnection: error with poll(), errno: "
             << theError);
        return -1;
    } // else 0 or +ve return from poll() is ok.

    if (p.revents & POLLIN || p.revents & POLLPRI) {
        // Data is ready to read...
        int connecting_socket = accept (listening_socket, NULL, NULL);
        if (connecting_socket < 0) {
            int theError = errno;
            INFO ("start-pollForConnection: Failed to accept on listening socket. errno: "
                 << theError);
            return -1;
        } // else connected ok.

        // Create a new connection instance and insert it into our map container
        SpineMLConnection* c = new SpineMLConnection();
        c->setConnectingSocket (connecting_socket);

        // Create a thread for this connection to operate in
        int rtn = pthread_create (&c->thread, NULL, &connectionThread, NULL);
        if (rtn < 0) {
            INFO ("start-pollForConnection: Failed to create connection thread.");
            return -1;
        }
        connections->insert (make_pair (c->thread, c)); // *** connectionThread needs to wait until
                                                        // this insert completes before getting on
                                                        // with its business.

        INFO ("start-pollForConnection: Accepted a connection.");

    } // else no new connections available on the listening socket

    return 0;
}

/*!
 * Does what it says on the tin. Clean up any connections which have
 * failed.
 */
void cleanupFailedConnections (void)
{
    map<pthread_t, SpineMLConnection*>::iterator connectionsIter = connections->begin();

    while (connectionsIter != connections->end()) {
        if (connectionsIter->second->getFailed() == true) {
            // allow failed connection to join
            pthread_join (connectionsIter->first, 0);
            // deallocate the SpineMLConnection object
            delete connectionsIter->second;
            // Remove pointer from map
            connections->erase (connectionsIter);
        }
        ++connectionsIter;
    }
}

/*!
 * Run through all the connections. If all of them have the "finished"
 * flag set, then set the connectionsFinished global flag to true.
 */
void checkForAllFinished (void)
{
    map<pthread_t, SpineMLConnection*>::iterator connIter = connections->begin();
    bool allFinished = true;
    // If there are no connections, we're probably at the start of the sequence:
    if (connIter == connections->end()) {
        allFinished = false;
    }
    while (connIter != connections->end()) {
        if (connIter->second->getFinished() == false) {
            allFinished = false;
            break;
        }
        ++connIter;
    }

    if (allFinished == true) {
        // Mark that all connections have now finished.
        connectionsFinished = true;
    }
}

/*
 * Delete connections.
 */
void deleteConnections (void)
{
    INFO ("start-deleteConnections: Cleaning up connections");

    map<pthread_t, SpineMLConnection*>::iterator connectionsIter = connections->begin();

    // First job - run through and allow all threads to join.
    while (connectionsIter != connections->end()) {
        pthread_join (connectionsIter->first, 0);
        ++connectionsIter;
    }

    // Now delete the connections
    connectionsIter = connections->begin();
    while (connectionsIter != connections->end()) {
        delete connectionsIter->second;
        ++connectionsIter;
    }

    // Clear, then delete the connections map.
    connections->clear();
    delete connections;
}

/*
 * thread function - this is where the TCP/IP comms happens. This is a
 * matlab-free zone. When this function exits, the SpineMLNet
 * environment is done with, and the matlab user will no longer be
 * able to transfer data to and from the SpineML experiment.
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
    while (!stopRequested && !connectionsFinished) {

        // First job in the loop is to see if we have any more
        // connections coming in from the client.
        retval = pollForConnection (listening_socket, p);
        if (retval != 0) {
            threadFinished = true;
            return NULL;
        }

        // Second job in loop is to do some housekeeping.
        cleanupFailedConnections();

        // Thirdly, check to see if all connections have
        // finished. This will set connectionsFinished and break us
        // out of this loop.
        checkForAllFinished();

        usleep (10000);

    } // while (!stopRequested && !connectionsFinished)

    // After finishing, if any connections are AM_SOURCE connections,
    // then we need to wait until the user has obtained the data. This
    // means we need another level of stop request. We need the thread
    // to be running for the user's mex "get data" functions to
    // operate. User also needs to find out if we've moved into this
    // state.
    if (!stopRequested) {
        if (haveAmSourceConnections()) {
            INFO ("start-theThread: Waiting for user to retrieve data.");
            while (!stopRequested) {
                usleep (10000);
            }
        }
    }

    // Shutdown code
    INFO ("start-theThread: Close sockets.");

    // Clean up.
    deleteConnections();
    INFO("Connections all deleted, now close listening socket " << listening_socket);
    closeSocket (listening_socket);

    threadFinished = true;

    INFO ("start-theThread: SpineMLNet environment is exiting.");
    return NULL;
}

#ifdef COMPILE_OCTFILE
DEFUN_DLD (spinemlnetStart, rhs, nlhs, "Start the spinemlnet server environment")
#else
void mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
#endif
{
    // initialise flags
    stopRequested = false;
    connectionsFinished = false;
    initialised = false;
    threadFinished = false;

    // Allocate the dataCache memory
    dataCache = new map<string, deque<double>*>();
    pthread_mutex_init (&dataCacheMutex, NULL);

    // init the mutex for our output debugging.
    coutMutex = new pthread_mutex_t;
    pthread_mutex_init (coutMutex, NULL);

#ifdef COMPILE_OCTFILE
    int nrhs = rhs.length();
#endif
    // Get port from the function args.
    if (nrhs>0) {
        // First argument to spinemlnetStart is the port number
#ifdef COMPILE_OCTFILE
        port = rhs(0).int_value();
#else
        port = (int)mxGetScalar(prhs[0]);
#endif
        if (port < 1) {
            INFO ("start-mexFunction: port " << port
                  << " is out of range, using default.");
            port = DEFAULT_PORT;
        }
        if (port > 65535) {
            // out of range
            INFO ("start-mexFunction: port " << port
                 << " is out of range, using default.");
            port = DEFAULT_PORT;
        }
        if (port < 1024) {
            // May have trouble running unless root
            INFO ("start-mexFunction: Warning: may need root access "
                 << "to serve on this port.");
        }
    } else {
        port = DEFAULT_PORT;
    }
    INFO ("start-mexFunction: Listening on port " << port);

    // create the thread
    INFO ("start-mexFunction: Creating SpineMLNet environment main thread.");
    int rtn = pthread_create (&thread, NULL, &theThread, NULL);
    if (rtn < 0) {
        INFO ("start-mexFunction: Failed to create main thread.");
#ifdef COMPILE_OCTFILE
        return octave_value_list();
#else
        return;
#endif
    }

    // wait until initialisation in the thread is complete before continuing
    do {
        usleep (1000);
        if (threadFinished == true) {
            // Shutdown as we have an error.
            INFO ("start-mexFunction: Shutting down due to error during initialisation.");
            // for each connection, delete it (thereby also destroying the mutexes):
            deleteConnections();
            // clear the loop
            initialised = true;
        }
    } while (!initialised);
    INFO ("start-mexFunction: SpineMLNet environment is initialised.");

    // details of output
#ifdef COMPILE_OCTFILE
    dim_vector dv(1, 2);
    dv(0) = 1; dv(1) = 16;
    uint64NDArray context(dv);

    // store thread information - this passed back to octave as context.
    context(0) = (unsigned long long int)&thread;
    context(1) = (unsigned long long int)&stopRequested;
    context(2) = (unsigned long long int)&threadFinished;
    context(3) = (unsigned long long int)connections;
    context(4) = (unsigned long long int)dataCache;
    context(5) = (unsigned long long int)&dataCacheMutex;
    context(6) = (unsigned long long int)coutMutex;
    context(7) = (unsigned long long int)&connectionsFinished;

    return octave_value (context);
#else
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
    context[6] = (unsigned long long int)coutMutex;
    context[7] = (unsigned long long int)&connectionsFinished;
#endif
}
