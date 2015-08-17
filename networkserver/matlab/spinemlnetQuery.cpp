/*
 * Query TCP/IP server thread. Return information about whether the
 * main TCP/IP server thread finished. This is returned in the first
 * element of a 1 by 2 matrix. A zero is currently returned in the
 * second element of this matrix. I've left this mex function
 * returning a matrix, rather than a scaler, in case I need to return
 * any additional information.
 */

#ifdef COMPILE_OCTFILE
# include <octave/oct.h>
#else
# ifdef __APPLE__
// To enable compilation on Mac OS X 10.8.
typedef unsigned short char16_t;
# endif
# include "mex.h"
#endif

#include <iostream>
#include <map>
#include <string.h>

extern "C" {
#include <pthread.h>
}

#include "SpineMLConnection.h"

pthread_mutex_t* coutMutex;

using namespace std;

#ifdef COMPILE_OCTFILE
DEFUN_DLD (spinemlnetQuery, rhs, nlhs, "Query the spinemlnet server environment")
#else
void
mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
#endif
{
#ifdef COMPILE_OCTFILE
    int nrhs = rhs.length();
#endif

    if (nrhs==0) {
#ifdef COMPILE_OCTFILE
        cerr << "Please pass in a context matrix as the first argument." << endl;
        return octave_value_list();
#else
        mexErrMsgTxt ("Please pass in a context matrix as the first argument.");
#endif
    }

    // FIXME: Need to check here that the pointer points to
    // something. What gets passed in if this mex function is
    // mis-called?

#ifdef COMPILE_OCTFILE
    uint64NDArray context = rhs(0).uint64_array_value();
    long unsigned int val = context(0);
    pthread_t *thread = (pthread_t*) val;
    val = context(1);
    volatile bool *stopRequested = (volatile bool*) val;
    val = context(2);
    volatile bool *threadFinished = (volatile bool*) val;
    val = context(6);
    coutMutex = (pthread_mutex_t*) val;
    val = context(7);
    volatile bool *connectionsFinished = (volatile bool*) val;
#else
    unsigned long long int *context; // Or some other type?
    context = (unsigned long long int*)mxGetData(prhs[0]);
    // This points to the main thread
    pthread_t *thread = ((pthread_t*) context[0]);
    volatile bool *stopRequested = ((volatile bool*) context[1]);
    // Has the main thread finished?
    volatile bool *threadFinished = ((volatile bool*) context[2]);
    coutMutex = (pthread_mutex_t*)context[6];
    volatile bool *connectionsFinished = ((volatile bool*) context[7]);
#endif


#ifdef COMPILE_OCTFILE
    dim_vector dv (1, 2);
    dv(0) = 1; dv(1) = 2;
    uint16NDArray lhs(dv);
    lhs(0,0) = (*threadFinished?1:0);
    lhs(0,1) = (*connectionsFinished?1:0);
    return octave_value (lhs);
#else
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
#endif
}
