/*
 * Query TCP/IP server thread. Return information about whether the
 * main TCP/IP server thread finished. This is returned in the first
 * element of a 1 by 2 matrix. A zero is currently returned in the
 * second element of this matrix. I've left this mex function
 * returning a matrix, rather than a scaler, in case I need to return
 * any additional information.
 */

// To enable compilation on Mac OS X 10.8
#ifndef char16_t
typedef unsigned short char16_t;
#endif
#include "mex.h"

#include <iostream>
#include <map>
#include <string.h>

extern "C" {
#include <pthread.h>
}

#include "SpineMLConnection.h"

pthread_mutex_t* coutMutex;

using namespace std;

void
mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    if (nrhs==0) {
        mexErrMsgTxt ("Please pass in a context matrix as the first argument.");
    }

    // FIXME: Need to check here that the pointer points to
    // something. What gets passed in if this mex function is
    // mis-called?

    unsigned long long int *context; // Or some other type?
    context = (unsigned long long int*)mxGetData(prhs[0]);
    // This points to the main thread
    pthread_t *thread = ((pthread_t*) context[0]);
    volatile bool *stopRequested = ((volatile bool*) context[1]);
    // Has the main thread finished?
    volatile bool *threadFinished = ((volatile bool*) context[2]);
    coutMutex = (pthread_mutex_t*)context[6];
    volatile bool *connectionsFinished = ((volatile bool*) context[7]);

    // The data for the output array. 2 numbers for now.
    const mwSize res[2] = { 1, 2 }; // 1 by 2 matrix

    // create the output array
    // args: ndims, const mwSize *dims, mxClassID classid, complexity flag
    plhs[0] = mxCreateNumericArray (2, res, mxUINT16_CLASS, mxREAL);

    // set up a pointer to the output array
    unsigned short *outPtr = (unsigned short*) mxGetData (plhs[0]);

    // copy new data into the output structure
    unsigned short rtn[2] = { (unsigned short)*threadFinished,
                              (unsigned short)*connectionsFinished };
    memcpy (outPtr, (const void*)rtn, 4); // 2 bytes per short * 2 elements in rtn.
}
