/*
 * Add data to a connection. Pass in both the name of the connection
 * (this is the name it is given in the SpineCreator experiment
 * interface) and the data to add to the connection.
 *
 * Returns a matrix containing: How much data is now in the
 * connection; any error (such as "this connection is no an AM_TARGET
 * connection" or "the named connection does not exist".
 */

#include "mex.h"
#include "matrix.h"
#include <iostream>
#include <map>
#include <deque>
#include <string.h>

extern "C" {
#include <pthread.h>
}

#include "SpineMLConnection.h"

using namespace std;

void
mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    if (nrhs!=3) {
        mexErrMsgTxt("Need 3 arguments");
    }

    unsigned long long int *context; // Or some other type?
    context = (unsigned long long int*)mxGetData(prhs[0]);
    // This points to the main thread
    pthread_t *thread = ((pthread_t*) context[0]);
    // Has the main thread finished?
    volatile bool *threadFinished = ((volatile bool*) context[2]);
    // Active connections
    map<pthread_t, SpineMLConnection*>* connections = (map<pthread_t, SpineMLConnection*>*) context[3];

    // NB: Don't name this local variable dataCache, else it will
    // clash with the one in the SpineMLConnection class.
    map<string, deque<double>*>* dCache = (map <string, deque<double>*>*) context[4];
    pthread_mutex_t* dCacheMutex = (pthread_mutex_t*)context[5];

    bool tf = *threadFinished; // Seems to be necessary to make a copy
                               // of the thing pointed to by
                               // threadFinished.

    // Get connection name from input arguments.
    char* targetConn = mxArrayToString(prhs[1]);
    //cout << "Target connection:  " << targetConn << endl;
    // Lazily stick the char array into a string:
    string targetConnection(targetConn); // get from input args
    mxFree (targetConn);

    // create a pointer to the real data in the input matrix
    // Use the mxGetN function to get the size of the matrix.
    double* inputData = mxGetPr(prhs[2]); // mxGetData for anything other than ptr to doubles.
    size_t ncols = mxGetN(prhs[2]);
    size_t nrows = mxGetM(prhs[2]);
    cout << "Input matrix has " << ncols << " cols and " << nrows << " rows.";

    string errormsg("");

    size_t dataLength = 0;
    // Validate input data:
    if (ncols > 1 && nrows > 1) { // Both > 1 - 2d matrix
        errormsg = "This mex function supports only one dimensional matrices.";
    } else if (ncols == 0 || nrows == 0) { // At least one is 0, no data
        errormsg = "No data passed in.";
    } else { // Both >0, but not both are greater than 1 - 1d matrix
        if (ncols > 1) {
            dataLength = ncols;
        } else if (nrows > 1) {
            dataLength = nrows;
        } else {
            // Both must be exactly 1
            dataLength = nrows;
        }
    }

    unsigned int dataSize = 0;

    if (!tf && errormsg.empty()) {

        bool added = false;

        // First, see if we can add the inputData to a connection.
        map<pthread_t, SpineMLConnection*>::iterator connIter = connections->begin();
        while (connIter != connections->end()) {
            if (connIter->second->getClientConnectionName() == targetConnection) {
                // Matched connection.
                connIter->second->addData (inputData, dataLength);
                // Find out how much data there is now.
                dataSize = connIter->second->getDataSize();

                added = true;
            }
            ++connIter;
        }

        // Second
        if (!added) {
            // There was no existing connection to add these data to;
            // add to the connectionData map instead.

            // Get dCache mutex
            pthread_mutex_lock (dCacheMutex);
            if (dCache != (map<string, deque<double>*>*)0) {
                map<string, deque<double>*>::iterator targ = dCache->find (targetConnection);
                if (targ != dCache->end()) {
                    // We already have data for that connection name; add to it.

                    // targ->second is a deque<double>*. We need a mutex on dataCache.
                    nrows = 0; // re-use as iterator
                    while (nrows < dataSize) {
                        targ->second->push_back (*inputData);
                        ++inputData;
                        ++nrows;
                    }

                    cout << "Inserted data into existing dataCache entry." << endl;
                } else {
                    // No existing cache of data for targetConnection.
                    deque<double>* dc = new deque<double>();
                    nrows = 0; // re-use as iterator
                    while (nrows < dataSize) {
                        dc->push_back (*inputData);
                        ++inputData;
                        ++nrows;
                    }

                    dCache->insert (make_pair (targetConnection, dc));
                    cout << "Inserted data into new dataCache entry." << endl;
                }
            } else {
                errormsg = "Can't add data, no established connection and no dataCache.";
            }
            // release dCache mutex
            pthread_mutex_unlock (dCacheMutex);
        }

    } else {
        errormsg = "Can't add data; the main thread has finished.";
    }

    // create the output array. For this mex function, we'll pass back
    // an error message, which may be an empty string if all was well,
    // along with the size of the data in the connection, if
    // applicable.

    char* output_buf = (char*) mxCalloc(errormsg.size()+1, sizeof(char));
    // Copy errormsg into output_buf
    strncpy (output_buf, errormsg.c_str(), errormsg.size());
    plhs[1] = mxCreateString (output_buf);

    // The caller works out how many left hand side args there are.
    //cout << "nlhs=" << nlhs << endl;

    // The data for the output array. 2 numbers for now.
    const mwSize res[2] = { 1, 2 }; // 1 by 2 matrix

    // args: ndims, const mwSize *dims, mxClassID classid, complexity flag
    //cout << "create numeric array..." << endl;
    plhs[0] = mxCreateNumericArray (2, res, mxUINT16_CLASS, mxREAL);

    // set up a pointer to the output array
    unsigned short *outPtr = (unsigned short*) mxGetData(plhs[0]);

    // copy new data into the output structure
    unsigned short rtn[2] = { (unsigned short)*threadFinished, dataSize };

    memcpy(outPtr, (const void*)rtn, 4); // 2 bytes per short * 2 elements in rtn.
}
