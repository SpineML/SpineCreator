/*
 * Query TCP/IP server thread.
 */

#include "mex.h"
#include <iostream>
#include <map>
#include <string.h>

#include "SpineMLConnection.h"

using namespace std;

void
mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    if (nrhs==0) {
        mexErrMsgTxt("failed");
    }

    // FIXME: Need to check here that the pointer points to
    // something. What gets passed in if this mex function is
    // mis-called?

    unsigned long long int *threadPtr; // Or some other type?
    threadPtr = (unsigned long long int*)mxGetData(prhs[0]);
    // This points to the main thread
    pthread_t *thread = ((pthread_t*) threadPtr[0]);
    // Has the main thread finished?
    volatile bool *threadFinished = ((volatile bool*) threadPtr[2]);
    // Active connections
    map<pthread_t, SpineMLConnection*>* connections = (map<pthread_t, SpineMLConnection*>*) threadPtr[3];

    // Pass in some numbers from matlab - if a vector, then it's
    // numbers for first connection. If >1 connection, it's numbers
    // for several connections.

    // sometype data = mxGetData(prhs[1]);

    // Extract these, and add them, as possible, to the Connections,
    // using name as index

    map<pthread_t, SpineMLConnection*>::iterator connIter = connections->begin();
    while (connIter != connections->end()) {
        cout << "Add data to a connection!" << endl;
        // Do stuff here - add some data to the deque:
        double d = 1.0;
        connIter->second->addNum(d);
        d += 1.0;
        connIter->second->addNum(d);
        d += 1.0;
        connIter->second->addNum(d);
        d += 1.0;
        connIter->second->addNum(d);

        ++connIter;
    }


    bool tf = *threadFinished; // Seems to be necessary to make a copy
                               // of the thing pointed to by
                               // threadFinished.

    // The data for the output array. 2 numbers for now.
    const mwSize res[2] = { 1, 2 }; // 1 by 2 matrix

    // create the output array
    // args: ndims, const mwSize *dims, mxClassID classid, complexity flag
    //cout << "create numeric array..." << endl;
    plhs[0] = mxCreateNumericArray (2, res, mxUINT16_CLASS, mxREAL);

    // set up a pointer to the output array
    unsigned short *outPtr = (unsigned short*) mxGetData(plhs[0]);

    // copy new data into the output structure
    unsigned short rtn[2] = { (unsigned short)*threadFinished, 0 };
    //cout << "memcpy..." << endl;
    memcpy(outPtr, (const void*)rtn, 4); // 2 bytes per short * 2 elements in rtn.
}
