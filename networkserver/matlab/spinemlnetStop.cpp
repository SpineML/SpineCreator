/*
 * Stop tcpip server.
 */

#include "mex.h"
#include <iostream>
#include <map>
#include <deque>

extern "C" {
#include <pthread.h>
}

pthread_mutex_t* coutMutex;

#include "SpineMLDebug.h"

using namespace std;

void
mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    cout << "SpineMLNet: stop-mexFunction: Called" << endl;
    unsigned long long int *context; // Or some other type?
    if (nrhs==0) {
       mexErrMsgTxt("failed");
    }

    context = (unsigned long long int*)mxGetData(prhs[0]);
    pthread_t *thread = ((pthread_t*) context[0]);
    volatile bool *stopRequested = ((volatile bool*) context[1]);
    map<string, deque<double>*>* dCache = (map <string, deque<double>*>*) context[4];
    pthread_mutex_t* dCacheMutex = (pthread_mutex_t*)context[5];
    coutMutex = (pthread_mutex_t*)context[6];

    // request termination
    *stopRequested = true;

    // Now see if we're in the "need to get data state", in which case
    // we don't want to wait for the join, or perhaps, we want to
    // force through (seeing as user pressed Ctrl-C).

    // Can check HERE if we have any data?


    // wait for thread to terminate
    cout << "SpineMLNet: stop-mexFunction: Wait for thread to join..." << endl;
    pthread_join(*thread, 0);

    // free the dataCache memory (allocated in spinemlnetStart.cpp)
    cout << "SpineMLNet: stop-mexFunction: deallocate dataCache memory" << endl;
    delete dCache;
    // And the mutex:
    pthread_mutex_destroy(dCacheMutex);

    pthread_mutex_destroy(coutMutex);
    delete coutMutex;

    cout << "SpineMLNet: stop-mexFunction: Returning" << endl;
}
