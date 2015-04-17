/*
 * Add data to a connection. Pass in both the name of the connection
 * (this is the name it is given in the SpineCreator experiment
 * interface) and the data to add to the connection.
 *
 * Returns a matrix containing: How much data is now in the
 * connection; any error (such as "this connection is no an AM_TARGET
 * connection" or "the named connection does not exist".
 *
 * The right way to call this in matlab-space is something like:
 *
 * [rtn errormsg] = spinemlnetAddData (context, 'realtime', [1.0 2.0 4.0 8.0])
 *
 * where context is the SpineMLNet context structure, as created by
 * spinemlnetStart, 'realtime' is the name of the connection and the
 * third argument is a one dimensional array of numbers (can be a row
 * or a column vector).
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

extern "C" {
#include <pthread.h>
}

#include "SpineMLConnection.h"

pthread_mutex_t* coutMutex;

using namespace std;

#ifdef COMPILE_OCTFILE
DEFUN_DLD (spinemlnetAddData, rhs, nlhs, // The arg 'rhs' is of type octave_value.
           "Add data to the spinemlnet server environment's data buffers")
#else
void
mexFunction (int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
#endif
{
#ifdef COMPILE_OCTFILE
    int nrhs = rhs.length();
#endif

    if (nrhs!=3) {
#ifdef COMPILE_OCTFILE
        cerr << "Need 3 arguments for this mex function." << endl;
        return octave_value_list();
#else
        mexErrMsgTxt ("Need 3 arguments for this mex function.");
#endif
    }


#ifdef COMPILE_OCTFILE
    uint64NDArray context = rhs(0).uint64_array_value();
    long unsigned int val = context(0);
    pthread_t *thread = (pthread_t*) val;
    val = context(1);
    volatile bool *stopRequested = (volatile bool*) val;
    val = context(2);
    volatile bool *threadFinished = (volatile bool*) val;
    val = context(3);
    map<pthread_t, SpineMLConnection*>* connections = (map<pthread_t, SpineMLConnection*>*) val;
    val = context(4);
    map<string, deque<double>*>* dCache = (map <string, deque<double>*>*) val;
    val = context(5);
    pthread_mutex_t* dCacheMutex = (pthread_mutex_t*) val;
    val = context(6);
    coutMutex = (pthread_mutex_t*) val;
#else
    unsigned long long int *context; // Or some other type?
    context = (unsigned long long int*)mxGetData (prhs[0]);
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

    // It's very important to get coutMutex set up from the context,
    // so that INFO() and DBG() calls won't crash matlab/octave.
    coutMutex = (pthread_mutex_t*)context[6];
#endif

    bool tf = *threadFinished; // Seems to be necessary to make a copy
                               // of the thing pointed to by
                               // threadFinished.

    // Get connection name from input arguments.
#ifdef COMPILE_OCTFILE
    string targetConnection = rhs(1).string_value();
#else
    char* targetConn = mxArrayToString(prhs[1]);
    // Lazily stick the char array into a string, as strings are easier to manipulate.
    string targetConnection(targetConn); // get from input args
    mxFree (targetConn);
#endif

    string errormsg("");

#ifdef COMPILE_OCTFILE
    const double* inputData = (const double*)0;
#else
    double* inputData = (double*)0;
#endif
    size_t ncols = 0, nrows = 0, inputDataLength = 0;

    // Get input data vector
#ifdef COMPILE_OCTFILE
    NDArray m = rhs(2).array_value();
    dim_vector dims = m.dims();
    nrows = dims(0);
    ncols = dims(1);
    if (ncols > 0 && nrows > 0) { // We have a matrix

        // Can we get a pointer to the underlying data in m?
        inputData = m.data();

        // Need to interleave the data as we have >1 row and therefore >1 time series.
        INFO ("Data for '" << targetConnection << "' contains " << nrows << " time series...");
        inputDataLength = nrows * ncols;

    } else { // At least one dimension is 0.
        errormsg = "No data passed in.";
    }
#else
    if (!mxIsDouble (prhs[2])) {
        errormsg = "Need doubles in the array";
    } else {
        inputData = mxGetPr(prhs[2]); // Would use mxGetData for anything other than a ptr to doubles.
        ncols = mxGetN(prhs[2]);
        nrows = mxGetM(prhs[2]);

        // Validate input data:
        if (ncols > 0 && nrows > 0) { // We have a matrix
            // Need to interleave the data as we have >1 row and therefore >1 time series.
            INFO ("Data for '" << targetConnection << "' contains " << nrows << " time series...");
            inputDataLength = nrows * ncols;

        } else { // At least one dimension is 0.
            errormsg = "No data passed in.";
        }
    }
#endif

    // This will be used to retrieve the amount of data stored in the connection.
    unsigned int connectionDataSize = 0;

    if (!tf && errormsg.empty()) {

        bool added = false;

        // First, see if we can add the inputData to a connection.
        map<pthread_t, SpineMLConnection*>::iterator connIter = connections->begin();
        while (connIter != connections->end()) {
            if (connIter->second->getClientConnectionName() == targetConnection) {
                // Matched connection.
                connIter->second->addData (inputData, inputDataLength);
                // Find out how much data there is now.
                connectionDataSize = connIter->second->getDataSize();

                added = true;
            }
            ++connIter;
        }

        // Second, if we've not been able to add to a connection, add to our dataCache.
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
                    while (nrows < inputDataLength) {
                        targ->second->push_back (*inputData);
                        ++inputData;
                        ++nrows;
                    }

                    INFO ("Inserted data (" << targ->second->size() << " doubles) into existing dataCache entry.");

                } else {
                    // No existing cache of data for targetConnection.
                    deque<double>* dc = new deque<double>();
                    nrows = 0; // re-use as iterator
                    while (nrows < inputDataLength) {
                        dc->push_back (*inputData);
                        ++inputData;
                        ++nrows;
                    }

                    dCache->insert (make_pair (targetConnection, dc));
                    INFO ("Inserted data (" << dc->size() << " doubles) into new dataCache entry.");
                }
            } else {
                errormsg = "Can't add data, no established connection and no dataCache.";
            }

            // release dCache mutex
            pthread_mutex_unlock (dCacheMutex);
        }

    } else if (tf && errormsg.empty()) {
        errormsg = "Can't add data; the main thread has finished.";
    } // else errormsg already contained a message.

    // create the output matrices. For this mex function, we'll pass
    // back an error message, which may be an empty string if all was
    // well, along with the size of the data in the connection, if
    // applicable.

    // plhs[0] takes the "threadFinished" bool, and the current data
    // size.

#ifdef COMPILE_OCTFILE
    dim_vector dv(1, 2); // Note: Produces compiler warning.
    dv(0) = 1; dv(1) = 2;
    uint16NDArray lhs(dv);
    lhs(0,0) = (*threadFinished?1:0);
    lhs(0,1) = connectionDataSize;
    octave_value_list return_vals;
    return_vals(0) = lhs;
    charMatrix ch(errormsg);
    return_vals(1) = octave_value(ch, '\'');
    return return_vals;
#else
    const mwSize res[2] = { 1, 2 }; // 1 by 2 matrix
    plhs[0] = mxCreateNumericArray (2, res, mxUINT16_CLASS, mxREAL);
    // set up a pointer to the output array
    unsigned short *outPtr = (unsigned short*) mxGetData (plhs[0]);
    // copy new data into the output structure
    unsigned short rtn[2] = { (unsigned short)*threadFinished, connectionDataSize };
    memcpy (outPtr, (const void*)rtn, 4); // 2 bytes per short * 2 elements in rtn.

    // plhs[1] takes the error message.
    if (nlhs > 1) {
        char* output_buf = (char*) mxCalloc (errormsg.size()+1, sizeof(char));
        strncpy (output_buf, errormsg.c_str(), errormsg.size());
        plhs[1] = mxCreateString (output_buf);
    }
#endif
}
