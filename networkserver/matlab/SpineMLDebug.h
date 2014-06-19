/* -*-c++-*- */

/*
 * Debug macros for SpineMLNet code.
 */

#ifndef _SPINEMLDEBUG_H_
#define _SPINEMLDEBUG_H_

extern "C" {
#include <pthread.h>
}

extern pthread_mutex_t* coutMutex;

#define INFO(s)                                 \
    pthread_mutex_lock (coutMutex);             \
    cout << "SpineMLNet: " << s << std::endl;   \
    pthread_mutex_unlock (coutMutex);

#ifdef DEBUG
#define DBG(s)                                  \
    pthread_mutex_lock (coutMutex);             \
    cout << "SpineMLNet: " << s << std::endl;   \
    pthread_mutex_unlock (coutMutex);
#else
# define DBG(s)
#endif


#endif // _SPINEMLDEBUG_H_
