#ifndef SENTENCE_H_
#define SENTENCE_H_

#include <string.h>
#include <pthread.h>
#include "SentenceFeatureValue.h";

class Sentence
{
    public:
        std::string sen_id;
        //this is to keep track which incidence this sentence is from in order to do the time based sampling
        int incidence_id;
        SentenceFeatureValue *featureValue;
        pthread_mutex_t mutex;
        Sentence(std::string sentenceid, SentenceFeatureValue *featureValue1, int incidence_id1)
        {
            sen_id = sentenceid;
            incidence_id = incidence_id1;
            featureValue = featureValue1;
            pthread_mutex_init(&mutex, NULL);
        }

        ~Sentence()
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

#endif // SENTENCE_H_
