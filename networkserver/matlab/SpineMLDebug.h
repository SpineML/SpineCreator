/* -*-c++-*- */

/*
 * Debug macros for SpineMLNet code.
 */

#ifndef _SPINEMLDEBUG_H_
#define _SPINEMLDEBUG_H_

extern "C" {
#include <pthread.h>
}

#ifndef DEBUG
#define DEBUG 1
#endif

extern pthread_mutex_t* coutMutex;

// INFO is for messages which should always be shown.
#define INFO(s)                                 \
    pthread_mutex_lock (coutMutex);             \
    cout << "SpineMLNet: " << s << std::endl;   \
    pthread_mutex_unlock (coutMutex);

// DBG is for debugging, but program should still be usable with this turned on.
#ifdef DEBUG
#define DBG(s)                                  \
    pthread_mutex_lock (coutMutex);             \
    cout << "SpineMLNet: " << s << std::endl;   \
    pthread_mutex_unlock (coutMutex);
#else
# define DBG(s)
#endif

// DBG2 is for horribly noisy stuff.
#ifdef DEBUG2
#define DBG2(s)                                 \
    pthread_mutex_lock (coutMutex);             \
    cout << "SpineMLNet: " << s << std::endl;   \
    pthread_mutex_unlock (coutMutex);
#else
# define DBG2(s)
#endif


#endif // _SPINEMLDEBUG_H_
