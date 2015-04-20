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

#ifdef COMPILE_OCTFILE
# include <octave/oct.h>
#else
# ifdef __APPLE__
// To enable compilation on Mac OS X 10.8.
typedef unsigned short char16_t;
# endif
# include "mex.h"
# include "matrix.h"
#endif

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

#ifdef COMPILE_OCTFILE
DEFUN_DLD (spinemlnetGetData, rhs, nlhs, "Get buffered data from the spinemlnet server environment")
#else
void
mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
#endif
{
#ifdef COMPILE_OCTFILE
    int nrhs = rhs.length();
#endif

    if (nrhs != 2) {
#ifdef COMPILE_OCTFILE
        cerr << "Need 2 arguments for this octfile function." << endl;
        return octave_value_list();
#else
        mexErrMsgTxt ("Need 2 arguments for this mex function.");
#endif
    }


#ifdef COMPILE_OCTFILE
    uint64NDArray context = rhs(0).uint64_array_value();
    long unsigned int val = context(0);
    pthread_t *thread = (pthread_t*) val;
    val = context(2);
    volatile bool *threadFinished = (volatile bool*) val;
    val = context(3);
    map<pthread_t, SpineMLConnection*>* connections = (map<pthread_t, SpineMLConnection*>*) val;
    val = context(6);
    coutMutex = (pthread_mutex_t*) val;
#else
    unsigned long long int *context;
    context = (unsigned long long int*)mxGetData (prhs[0]);
    pthread_t *thread = ((pthread_t*) context[0]);
    volatile bool *threadFinished = ((volatile bool*) context[2]);
    // Active connections
    map<pthread_t, SpineMLConnection*>* connections = (map<pthread_t, SpineMLConnection*>*) context[3];
    // set up coutMutex for INFO() and DBG() calls.
    coutMutex = (pthread_mutex_t*)context[6];
#endif
    bool tf = *threadFinished;

    // Get connection name from input arguments.
#ifdef COMPILE_OCTFILE
    string targetConnection = rhs(1).string_value();
#else
    char* targetConn = mxArrayToString(prhs[1]);
    string targetConnection(targetConn);
    mxFree (targetConn);
#endif

    string errormsg("");
    unsigned int connectionDataSize = 0;
    bool gotdata = false, gotmatch = false, notready = false;

#ifdef COMPILE_OCTFILE
    NDArray lhs;
# ifdef ANNOUNCE_VERSION
    INFO ("octfile-version of getdata for target connection '" << targetConnection << "'....");
# endif
#endif

    if (!tf) {

        // First, see if we can add the inputData to a connection.
        map<pthread_t, SpineMLConnection*>::iterator connIter = connections->begin();
        if (connIter == connections->end()) {
            errormsg = "No connections available.";
        }
        while (connIter != connections->end()) {
            if (connIter->second->getClientConnectionName() == targetConnection) {

                DBG2 ("Matched connection!");
                gotmatch = true;

                // Test if the connection is neither established nor
                // finished, in which case it's a matched connection
                // which is not yet ready to return data.
                if (connIter->second->getEstablished() == false
                    && connIter->second->getFinished() == false) {
                    // We got a matched connection, but it's not ready yet, so break out.
                    notready = true;
                    break;
                }

                // Find out how much data there is in the matched connection.
                connectionDataSize = connIter->second->getDataSize();
                DBG2 ("connectionDataSize: " << connectionDataSize);
                // We'll make a clientDataSize by connectionDataSize/clientDataSize matrix.
                unsigned int matrixRows = connIter->second->getClientDataSize();
                unsigned int matrixCols = connectionDataSize/matrixRows;
                DBG2 ("rows: " << matrixRows << " cols: " << matrixCols);
                // Now need to copy this data into our output.
                unsigned int i = 0;
#ifdef COMPILE_OCTFILE
                dim_vector datadv(1, 2);
                datadv(0) = matrixRows; datadv(1) = matrixCols;
                lhs.resize(datadv);
                octave_idx_type row_idx = 0;
                octave_idx_type col_idx = 0;
                while (i < connectionDataSize && col_idx < matrixCols) {
                    while (i < connectionDataSize && row_idx < matrixRows) {
                        lhs (row_idx, col_idx) = connIter->second->popFront();
                        ++i; ++row_idx;
                    }
                    ++col_idx;
                    row_idx = 0;
                }
#else
                const mwSize res[2] = { (int)matrixRows, (int)matrixCols };
                plhs[0] = mxCreateNumericArray (2, res, mxDOUBLE_CLASS, mxREAL);
                // set up a pointer to the output array
                double* outPtr = (double*) mxGetData (plhs[0]); // plhs[0] is an mxArray.
                // copy new data into the output structure
                while (i < connectionDataSize) {
                    try {
                        // This just works, whatever the dimensionality of the matrix:
                        *outPtr++ = connIter->second->popFront();
                    } catch (std::out_of_range& e) {
                        break;
                    }
                    ++i;
                }
#endif
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
            } else if (notready) {
                errormsg = "Matching connection not ready.";
            } else {
                errormsg = "Connection contained no data.";
            }
        }
        DBG2 ("spinemlnetGetData: Failed to get data: " << errormsg);

#ifdef COMPILE_OCTFILE
        // Insert 2 zeros so we return a matrix containing nothing:
        dim_vector dv(1, 2);
        dv(0) = 1; dv(1) = 2;
        lhs.resize(dv);
        lhs(0,0) = 0; lhs(0,1) = 0;
#else
        const mwSize res[2] = { 1, 2 }; // 1 by 2 matrix
        plhs[0] = mxCreateNumericArray (2, res, mxDOUBLE_CLASS, mxREAL);
        // set up a pointer to the output array
        double* outPtr = (double*) mxGetData (plhs[0]);
        // Fill with 2 zeros
        *outPtr++ = 0; *outPtr = 0;
#endif
    }

#ifdef COMPILE_OCTFILE
    return octave_value (lhs);
#endif
}
