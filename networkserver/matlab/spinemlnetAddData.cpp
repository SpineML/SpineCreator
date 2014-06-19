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

    bool tf = *threadFinished; // Seems to be necessary to make a copy
                               // of the thing pointed to by
                               // threadFinished.

    // Get connection name from input arguments.
    char* targetConn = mxArrayToString(prhs[1]);
    cout << "Target connection:  " << targetConn << endl;
    // Lazily stick the char array into a string:
    string targetConnection(targetConn); // get from input args
    mxFree (targetConn);

    // create a pointer to the real data in the input matrix
    // Use the mxGetN function to get the size of the matrix.
    double* inMatrix = mxGetPr(prhs[2]); // mxGetData for anything other than ptr to doubles.
    size_t ncols = mxGetN(prhs[2]);
    size_t nrows = mxGetM(prhs[2]);
    cout << "Input matrix has " << ncols << " cols and " << nrows << " rows.";

    string errormsg("");
    unsigned int dataSize = 0;

    if (!tf) {

        // Pass in some numbers from matlab - if a vector, then it's
        // numbers for first connection. If >1 connection, it's numbers
        // for several connections.

        // sometype data = mxGetData(prhs[1]);

        // Extract these, and add them, as possible, to the Connections,
        // using name as index

        bool added = false;

        map<pthread_t, SpineMLConnection*>::iterator connIter = connections->begin();
        while (connIter != connections->end()) {
            if (connIter->second->getClientConnectionName() == targetConnection) {
                // Matched connection.

                // FIXME - this is dummy data being added.
                double d = 1.0;
                connIter->second->addNum(d);
                d += 1.0;
                connIter->second->addNum(d);

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
            if (dCache != (map<string, deque<double>*>*)0) {
                map<string, deque<double>*>::iterator targ = dCache->find (targetConnection);
                if (targ != dCache->end()) {
                    // We already have data for that connection name; add to it.
                    targ->second->push_back (7.0); // FIXME: Example code.
                    targ->second->push_back (8.0);
                    cout << "Inserted data into existing dataCache entry." << endl;
                } else {
                    // No existing cache of data for targetConnection.
                    deque<double>* dc = new deque<double>();
                    dc->push_back (7.0); // FIXME: Example code.
                    dc->push_back (8.0);
                    dCache->insert (make_pair (targetConnection, dc));
                    cout << "Inserted data into new dataCache entry." << endl;
                }
            } else {
                errormsg = "Can't add data, no established connection and no dataCache.";
            }
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
