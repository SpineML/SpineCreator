/*
 * Stop tcpip server.
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

extern "C" {
#include <pthread.h>
}

pthread_mutex_t* coutMutex;

#include "SpineMLDebug.h"

using namespace std;

#ifdef COMPILE_OCTFILE
DEFUN_DLD (spinemlnetStop, rhs, nlhs, "Stop the spinemlnet server environment")
#else
void
mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
#endif
{
    cout << "SpineMLNet: stop-" << __FUNCTION__<< ": Called" << endl;

#ifdef COMPILE_OCTFILE
    int nrhs = rhs.length();
#endif

    if (nrhs==0) {
#ifdef COMPILE_OCTFILE
        cerr << "failed" << endl;
        return octave_value_list();
#else
        mexErrMsgTxt("failed");
#endif
    }

#ifdef COMPILE_OCTFILE
    uint64NDArray context = rhs(0).uint64_array_value();
    long unsigned int val = context(0);
    pthread_t *thread = (pthread_t*) val;
    val = context(1);
    volatile bool *stopRequested = (volatile bool*) val;
    val = context(4);
    map<string, deque<double>*>* dCache = (map <string, deque<double>*>*) val;
    val = context(5);
    pthread_mutex_t* dCacheMutex = (pthread_mutex_t*) val;
    val = context(6);
    coutMutex = (pthread_mutex_t*) val;
#else
    unsigned long long int* context = (unsigned long long int*)mxGetData(prhs[0]);
    pthread_t *thread = ((pthread_t*) context[0]);
    volatile bool *stopRequested = ((volatile bool*) context[1]);
    map<string, deque<double>*>* dCache = (map <string, deque<double>*>*) context[4];
    pthread_mutex_t* dCacheMutex = (pthread_mutex_t*)context[5];
    coutMutex = (pthread_mutex_t*)context[6];
#endif

    // request termination
    *stopRequested = true;

    // Now see if we're in the "need to get data state", in which case
    // we don't want to wait for the join, or perhaps, we want to
    // force through (seeing as user pressed Ctrl-C).

    // Can check HERE if we have any data?


    // wait for thread to terminate
    cout << "SpineMLNet: stop-" << __FUNCTION__<< ": Wait for thread to join..." << endl;
    pthread_join(*thread, 0);

    // free the dataCache memory (allocated in spinemlnetStart.cpp)
    cout << "SpineMLNet: stop-" << __FUNCTION__<< ": deallocate dataCache memory" << endl;
    delete dCache;
    // And the mutex:
    pthread_mutex_destroy(dCacheMutex);

    pthread_mutex_destroy(coutMutex);
    delete coutMutex;

    cout << "SpineMLNet: stop-" << __FUNCTION__<< ": Returning" << endl;

#ifdef COMPILE_OCTFILE
    return octave_value_list();
#endif
}
