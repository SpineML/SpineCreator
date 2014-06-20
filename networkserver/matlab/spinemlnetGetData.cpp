/*
 * Get data from a connection. Pass in the name of the connection
 * (this is the name it is given in the SpineCreator experiment
 * interface). Naturally, the context also has to be passed in.
 *
 * Returns a matrix containing the data.
 *
 * The right way to call this in matlab-space is something like:
 *
 * mydata = spinemlnetGetData (context, 'realtime');
 *
 * where context is the SpineMLNet context structure, as created by
 * spinemlnetStart and 'realtime' is the name of the connection.
 */

#include "mex.h"
#include "matrix.h"
#include <iostream>
#include <map>
#include <deque>
#include <string.h>
#include <stdexcept>

extern "C" {
#include <pthread.h>
}

#include "SpineMLConnection.h"

pthread_mutex_t* coutMutex;

using namespace std;

void
mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    if (nrhs != 2) {
        mexErrMsgTxt ("Need 2 arguments for this mex function.");
    }

    unsigned long long int *context;
    context = (unsigned long long int*)mxGetData (prhs[0]);
    pthread_t *thread = ((pthread_t*) context[0]);
    volatile bool *threadFinished = ((volatile bool*) context[2]);
    bool tf = *threadFinished;
    // Active connections
    map<pthread_t, SpineMLConnection*>* connections = (map<pthread_t, SpineMLConnection*>*) context[3];
    // set up coutMutex for INFO() and DBG() calls.
    coutMutex = (pthread_mutex_t*)context[6];

    // Get connection name from input arguments.
    char* targetConn = mxArrayToString(prhs[1]);
    string targetConnection(targetConn);
    mxFree (targetConn);

    string errormsg("");
    unsigned int connectionDataSize = 0;
    bool gotdata = false, gotmatch = false;
    if (!tf) {

        // First, see if we can add the inputData to a connection.
        map<pthread_t, SpineMLConnection*>::iterator connIter = connections->begin();
        if (connIter == connections->end()) {
            errormsg = "No connections available.";
        }
        while (connIter != connections->end()) {
            if (connIter->second->getClientConnectionName() == targetConnection) {

                gotmatch = true;
                // Find out how much data there is in the matched connection.
                connectionDataSize = connIter->second->getDataSize();

                // Now need to copy this data into our output.
                const mwSize res[2] = { 1, connectionDataSize }; // 1 by 2 matrix
                plhs[0] = mxCreateNumericArray (2, res, mxDOUBLE_CLASS, mxREAL);
                // set up a pointer to the output array
                double* outPtr = (double*) mxGetData (plhs[0]);
                // copy new data into the output structure
                unsigned int i = 0;
                while (i < connectionDataSize) {
                    try {
                        *outPtr++ = connIter->second->popFront();
                    } catch (std::out_of_range& e) {
                        break;
                    }
                    ++i;
                }
                if (i>0) {
                    gotdata = true;
                }
                break;
            }
            ++connIter;
        }

    } else {
        errormsg = "Can't get data; the main thread has finished.";
    }

    if (!gotdata) {
        if (errormsg.empty()) {
            if (!gotmatch) {
                errormsg = "No matching connection found.";
            } else {
                errormsg = "Connection contained no data.";
            }
        }
        INFO ("spinemlnetGetData: Failed to get data: " << errormsg);
        const mwSize res[2] = { 1, 2 }; // 1 by 2 matrix
        plhs[0] = mxCreateNumericArray (2, res, mxDOUBLE_CLASS, mxREAL);
        // set up a pointer to the output array
        double* outPtr = (double*) mxGetData (plhs[0]);
        // Fill with 2 zeros
        *outPtr++ = 0; *outPtr = 0;
    }
}
