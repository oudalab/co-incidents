#ifndef SHAREDRESOURCES_H_
#define SHAREDRESOURCES_H_


#include <pthread.h>

class SharedResources
{

    public:
        int lastActiveIncidenceIndex;
        bool jumpout = false;
        pthread_mutex_t mutex;
        SharedResources(int lastActiveIncidenceIndex1)
        {
            lastActiveIncidenceIndex = lastActiveIncidenceIndex1;
            pthread_mutex_init(&mutex, NULL);
        }
        ~SharedResources()
        {
            pthread_mutex_destroy(&mutex);
        }

        void lock()
        {
            pthread_mutex_lock(&mutex);
        }

        void unlock()
        {
            pthread_mutex_unlock(&mutex);
        }
};


#endif // SHAREDRESOURCES_H_
