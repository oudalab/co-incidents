#ifndef INCIDENCE_H_
#define INCIDENCE_H_

#include <string>
#include <utility>
#include <vector>
#include <pthread.h>
#include "IncidenceFeature.h"


class Incidence
{
    public:
        int inci_id; //should be the index in the incidence array so each time need to update it when something changed.
        std::string sup_id;
        pthread_mutex_t mutex;
        /*will be a list of snetence id that is in the incidence*/
        std::vector<int> sentencesid;
        std::vector<std::string> subincidencesid;
        IncidenceFeature featureMap;
        Incidence(int incidenceid, std::vector<int> sentences): inci_id(incidenceid), sentencesid(std::move(sentences))
    {
        pthread_mutex_init(&mutex, NULL);
    }
        ~Incidence()
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

#endif // INCIDENCE_H_


