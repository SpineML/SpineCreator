/*
 * Stop tcpip server.
 */

#include "mex.h"
#include <iostream>
#include <map>
#include <deque>

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

    // request termination
    *stopRequested = true;
    // wait for thread to terminate
    cout << "SpineMLNet: stop-mexFunction: Wait for thread to join..." << endl;
    pthread_join(*thread, 0);

    // free the dataCache memory (allocated in spinemlnetStart.cpp)
    cout << "SpineMLNet: stop-mexFunction: deallocate dataCache memory" << endl;
    delete dCache;

    cout << "SpineMLNet: stop-mexFunction: Returning" << endl;
}
