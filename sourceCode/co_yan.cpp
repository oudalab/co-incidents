#include <iostream>
#include <pthread.h>
#include <string>
#include <ctime>
#include <vector>
#include <fstream>
#include <stdlib.h>
#include <algorithm>
#include <array>
#include <math.h>
#include <random>       // std::default_random_engine
#include <chrono>       // std::chrono::system_clock
//#include <uuid/uuid.h>
#include <map>
#include <cstdlib>
//load json files of features from disk
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/writer.h>

#include <mutex>
#include <thread>
#include <pthread.h>
//this the dimenstion of the word embedding.
#define EMBED_SIZE 300
//#define ITERATION 10000000
using namespace std;
int lastActiveIncidenceIndex = 0;
// Ctrl+Shift+Alt+Q: Quick Format.
// Ctrl+Shift+Alt+S: Selected Format.

void rtrim(std::string &);

//to track if the feature turned on or off on each incidence, if incidence feature has a value it means it is turned on, otherwise it means it is turned off.
class IncidenceFeature
{
public:
    map<string, string> featureMap;
    IncidenceFeature()
    {
        //all the feature should be turned on at the beginning
        //all the names here are the same as the json file, if it has value means it is turned on, if it is "" string, means this feature is turned off
        featureMap["code"] = "";
        featureMap["country_code"] = "";
        featureMap["date8"] = "";
        featureMap["geoname"] = "";
        featureMap["id"] = "";
        featureMap["year"] = "";
        featureMap["latitude"] = "";
        featureMap["longitude"] = "";
        featureMap["src_actor"] = "";
        featureMap["src_agent"] = "";
        featureMap["tgt_actor"] = "";
        featureMap["tgt_agent"] = "";
        //featureMap["month"]="";
        //featureMap["day"]="";
    }
};



class SentenceFeatureValue
{
public:
    //map<string,string> featureValue;
    string code;
    string rootcode;
    //string country_code;
    string date8;
    //string geoname;
    string id;
    string year;
    //string latitude;
    //string longitude;
    string src_actor;
    string src_agent;
    string tgt_actor;
    string tgt_agent;
    string month;
    string day;
    int *embed;
    //this will be the index in the global sentence array
    int index;
    // string doc;
    //string embed;
    SentenceFeatureValue(string code1, string rootcode1, string date81,  string id1, string year1, string src_actor1, string src_agent1, string tgt_actor1, string tgt_agent1, string month1, string day1, int *embed1, int index1)
    {
        code = code1;
        rootcode = rootcode1;
        //country_code = country_code1;
        date8 = date81;
        //geoname = geoname1;
        id = id1;
        year = year1;
        //latitude = latitude1;
        //longitude = longitude1;
        src_actor = src_actor1;
        src_agent = src_agent1;
        tgt_actor = tgt_actor1;
        tgt_agent = tgt_agent1;
        //doc=doc1;
        month = month1;
        day = day1;
        embed = embed1;
        index = index1;
        trimall();
    };
private:
    void trimall()
    {
        rtrim(code );
        rtrim(rootcode);
        //rtrim(country_code );
        rtrim(date8 );
        //rtrim(geoname );
        rtrim(id );
        rtrim(year);
        rtrim(month);
        rtrim(day);
        //rtrim(latitude );
        //rtrim(longitude );
        rtrim(src_actor );
        rtrim(src_agent );
        rtrim(tgt_actor );
        rtrim(tgt_agent );
    }
};

class GlobalFeatureWeight
{
public:
    map<string, int> featureWeight;
    GlobalFeatureWeight()
    {
        featureWeight["code"] = 1;
        featureWeight["root_code"] = 1;
        featureWeight["country_code"] = 1;
        featureWeight["date8"] = 1;
        featureWeight["geoname"] = 1;
        featureWeight["id"] = 1;
        featureWeight["year"] = 1;
        featureWeight["latitude"] = 1;
        featureWeight["longitude"] = 1;
        featureWeight["src_actor"] = 1;
        featureWeight["src_agent"] = 1;
        featureWeight["tgt_actor"] = 1;
        featureWeight["tgt_agent"] = 1;
        featureWeight["tgt_year"] = 1;
    }
};
class Sentence
{
public:
    string sen_id;
    SentenceFeatureValue *featureValue;
    pthread_mutex_t mutex;
    Sentence(string sentenceid, SentenceFeatureValue *featureValue1)
    {
        sen_id = sentenceid;
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

class SharedResources
{

public:
    int lastActiveIncidenceIndex;
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

class Incidence
{
public:
    string inci_id;
    string sup_id;
    pthread_mutex_t mutex;
    /*will be a list of snetence id that is in the incidence*/
    vector<int> sentencesid;
    vector<string> subincidencesid;
    IncidenceFeature featureMap;
    Incidence(string incidenceid, vector<int> sentences): inci_id(incidenceid), sentencesid(move(sentences))
    {
        pthread_mutex_init(&mutex, NULL);
    };
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

class Subincidence
{
public:
    //it hard to generate guid in c++, so maybe Subincidence don't need a guid
    string sub_id;
    string inci_id;
    vector<string> sentencesid;
    /*****list of features that subincidence care about*/;
    Subincidence(string subid, string inciid): sub_id(subid), inci_id(inciid) {}
};

class SuperIncidence
{
public:
    string sup_id;
    /* nodeid when distributed which node this superincidence is on*/
    string nodeid;
    /****list of freatures that super incidence care about***/
};

//global variables
int xlength = 1000;
int length = (xlength * xlength - xlength) / 2 + xlength;


int dotProduct(int *vect_A, int *vect_B)
{

    int product = 0;

    // Loop for calculate cot product
    for (int i = 0; i < EMBED_SIZE; i++)

        product = product + vect_A[i] * vect_B[i];
    return product;
}

double vectorLength(int *vect)
{
    double length = 0.0;
    for (int i = 0; i < EMBED_SIZE; i++)
    {
        length = length + (vect[i] * 1.0) * (vect[i] * 1.0);
    }
    return sqrt(length);
}


double cosineSimilarity(int *vec1, int *vec2)
{
    return (dotProduct(vec1, vec2) * 1.0) / (vectorLength(vec1) * vectorLength(vec2));
}

/*****generate a random number within a range,  include min and max value *****/
int generateRandomInteger(int min, int max)
{
    srand(std::chrono::system_clock::now().time_since_epoch().count());
    return min + (rand() % static_cast<int>(max - min + 1));
}

//give an integer split it into several other integers randomly and will add up to the integer
vector<int> *splitTheIntegerIntoRandomPart(int sum)
{
    vector<int> *randomNumberArray = new vector<int>();
    int numberGenerate = 0;
    while(sum > 0)
    {
        numberGenerate = generateRandomInteger(0, sum);
        //continue if the numberGenerated is 0, since that is useless
        if(numberGenerate == 0)
        {
            continue;
        }
        (*randomNumberArray).push_back(numberGenerate);
        sum = sum - numberGenerate;

    }
    return randomNumberArray;
}


vector<int> &shuffleTheIndexOfVector(int n)
{
    //shuffle the number ={0,1,2,3.....n-1}
    //std::array<int,5> foo={1,2,3,4,5};
    vector<int> *random = new vector<int>();
    for(int i = 0; i < n; i++)
    {
        (*random).push_back(i);
    }
    // obtain a time-based seed:
    srand(std::chrono::system_clock::now().time_since_epoch().count());
    random_shuffle((*random).begin(), (*random).end());
    cout << "startShuffle:" << endl;
    for (int &x : (*random))
        cout << x << endl;

    return (*random);
}

//split the target incidence if new stuff get added in ,
//the current thought might need to change later.
// void splitIncidenceIntoSubincidence(int incidenceIndex, string incidenceId, vector<Subincidence *> &subincidenceArray, vector<Incidence *> &incidenceArray)
// {

//     //randomly split the sn
//     vector<int> sentences = (*incidenceArray[incidenceIndex]).sentencesid;
//     int sentencesCount = sentences.size();
//     vector<int> randomArray = *splitTheIntegerIntoRandomPart(sentencesCount);
//     //might give it an probablity to split or not.
//     //first shuffle the list then, make them in to subgroup, shuffle is provided by c++ native lib.
//     vector<int> shuffledIndex = shuffleTheIndexOfVector(sentencesCount);
//     int sizeid = 0;
//     int accumulateIndex = 0;
//     int sentenceIndex = 0;
//     string subid = "";
//     for(int num : randomArray)
//     {
//         sizeid = subincidenceArray.size();
//         subid = to_string(sizeid);
//         Subincidence *sub = new Subincidence(subid, incidenceId);
//         subincidenceArray.push_back(sub);
//         for(int i = 0; i < num; i++)
//         {

//             sentenceIndex = shuffledIndex[accumulateIndex];
//             (*sub).sentencesid.push_back(sentences[sentenceIndex]);
//             //be careful this needs to be called at last, otherwise it will get a segmentation fault
//             accumulateIndex = accumulateIndex + 1;
//         }
//         (*incidenceArray[incidenceIndex]).subincidencesid.push_back(subid);

//     }
//     cout << "subcount: " << (*incidenceArray[incidenceIndex]).subincidencesid.size() << endl;


// }


/**link a sentence to a coincidence*, when bigger than the threshold the stuff should be moved*/
/***array is by default pass by reference**/
// void linkSentenceToIncidence(int desincidenceindex, string incidenceid, int sourceincidenceindex, string sourceincidenceid, string sentenceid, int indexOfSentenceId, double threshold, vector<Incidence *> &incidenceArray, vector<Subincidence *> &subincidenceArray)
// {
//     //let say when the sentece similarity within the average similarity for the coincidence is above some value then move.
//     double sentenceWithIncidenceSimilarity = 0;
//     double similarityInOldIncidence = 0;
//     vector<string> sentencesid = (*(incidenceArray[stoi(incidenceid)])).sentencesid;
//     //int count=0;
//     int sen1 = 0;
//     int sen2 = stoi(sentenceid);
//     //in order for the proces to start do this:
//     if(sentencesid.size() == 1)
//     {
//         sentenceWithIncidenceSimilarity = generateRandomInteger(0, 100) / 100.0;
//     }
//     else
//     {
//         sentenceWithIncidenceSimilarity = 0;
//         for(string id : sentencesid)
//         {
//             sen1 = stoi(id);
//             //count=count+1;
//             //pairwisely calculate the similarity of the current add in stuff with the snetence already in the list and compare the similarity with some threshhold.
//             sentenceWithIncidenceSimilarity = sentenceWithIncidenceSimilarity + getSimilarityByMatrixIndex(matrix, sen1, sen2);
//         }

//     }
//     //now need to calculat sen2 affinity within its old icnidence
//     vector<string> sourceSentencesid = (*(incidenceArray[stoi(sourceincidenceid)])).sentencesid;
//     if(sourceSentencesid.size() == 0)
//     {
//         similarityInOldIncidence = generateRandomInteger(0, 100) / 100.0;
//     }
//     else
//     {
//         similarityInOldIncidence = 0;
//         for(string id : sourceSentencesid)
//         {
//             sen1 = stod(id);
//             similarityInOldIncidence = similarityInOldIncidence + getSimilarityByMatrixIndex(matrix, sen1, sen2);
//         }

//     }

//     //if(sentenceWithIncidenceSimilarity/count>=threshold)
//     if(sentenceWithIncidenceSimilarity >= similarityInOldIncidence)
//     {
//         //if bigger than threshhold, then link it
//         cout << "linked!!" << endl;
//         //remove from the old incidence and add into the new incidence.

//         vector<string> &sentenceids = (*(incidenceArray[sourceincidenceindex])).sentencesid;
//         sentenceids.erase(sentenceids.begin() + indexOfSentenceId);
//         //if there is no sentence inside of the incidence any more, remove the incidence from the incidence list
//         if(sentenceids.size() == 0)
//             incidenceArray.erase(incidenceArray.begin() + sourceincidenceindex);

//         //add the sentence to the destination incidence

//         (*(incidenceArray[desincidenceindex])).sentencesid.push_back(sentenceid);
//         splitIncidenceIntoSubincidence(desincidenceindex, incidenceid, subincidenceArray, incidenceArray);

//     }
//     else
//     {
//         cout << "not linked!!" << endl;
//     }
//     //now need to make the linked sentecnes into the incidence list, and need to get rid of the incidence if there is nothing belong to it any more,

// }
double getSimilarityBySentenceId( vector<Sentence *> &sentenceArray, int sen1index, int sen2index)
{
    int *vec1 = (*((*(sentenceArray[sen1index])).featureValue)).embed;
    int *vec2 = (*((*(sentenceArray[sen2index])).featureValue)).embed;
    double cosine = cosineSimilarity(vec1, vec2);
    return cosine;
}

double getSentenceSimilarityWithinIncidence(vector<Sentence *> &sentenceArray, vector<Incidence *> &incidenceArray, int incidenceid, int sentenceindex, bool fromsource)
{
    double sentenceWithIncidenceSimilarity = 0;
    // double similarityInOldIncidence = 0;
    vector<int> sentencesid = (*(incidenceArray[incidenceid])).sentencesid;
    //int count=0;
    int sen1 = 0;
    int sen2 = sentenceindex;
    //in order for the proces to start do this:
    if(sentencesid.size() == 1 && fromsource)
    {
        sentenceWithIncidenceSimilarity = generateRandomInteger(0, 55) / 100.0;
        return sentenceWithIncidenceSimilarity;
    }
    else
    {
        for(int id : sentencesid)
        {
            if(id != sentenceindex)
            {
                sen1 = id;
                //count=count+1;
                //pairwisely calculate the similarity of the current add in stuff with the snetence already in the list and compare the similarity with some threshhold.
                sentenceWithIncidenceSimilarity = sentenceWithIncidenceSimilarity + getSimilarityBySentenceId(sentenceArray, sen1, sen2);
                // if(sentenceWithIncidenceSimilarity>=0.5)
                // {
                // 	cout<<"dest incidence sen:"<<sen1<<endl;
                // 	cout<<"source incidence sen:"<<sen2<<endl;
                // }
            }
        }
    }
    if(!fromsource)
    {
        return sentenceWithIncidenceSimilarity / sentencesid.size();
    }
    else
    {
        return sentenceWithIncidenceSimilarity / (sentencesid.size() - 1);
    }


}
void rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch)
    {
        return !std::isspace(ch);
    }).base(), s.end());
}

bool isTrival(string input)
{
    // iinput.erase(std::remove(input.begin(), input.end(), '\n'), input.end());
    rtrim(input);
    if(input == "\"\"" || input == "" || input == " " || input.empty() || input.length() == 0)
    {
        return true;
    }
    return false;
}

//given two sentence check how many pairs property match
int getMatchWithinSentences(SentenceFeatureValue &feature1, SentenceFeatureValue &feature2, int score_threshold, map<string, int> &weightMap)
{
    int score = 0;
    int up = 2;

    if(!isTrival(feature1.code) && !isTrival(feature2.code) && feature1.code == feature2.code)
    {
        //cout<<"faatyre start"<<endl;
        //cerr<<"(" << feature1.code << ", " << feature2.code<< ") " << score << endl;
        score += up * weightMap["code"];
    }
    if(!isTrival(feature1.rootcode) && !isTrival(feature2.rootcode) && feature1.rootcode == feature2.rootcode)
    {
        //cout<<"faatyre start"<<endl;
        //cerr<<"(" << feature1.code << ", " << feature2.code<< ") " << score << endl;
        score += up * weightMap["rootcode"];
    }
    // if(!isTrival(feature1.country_code) && !isTrival(feature2.country_code) && feature1.country_code == feature2.country_code)
    // {
    //     //cerr<<"(" << feature1.country_code << ", " << feature2.country_code<< ") " << score << endl;
    //     //code += 2;
    //     score += up;
    // }
    if(!isTrival(feature1.year) && !isTrival(feature2.year) && feature1.year == feature2.year)
    {
        //cerr<<"(" << feature1.country_code << ", " << feature2.country_code<< ") " << score << endl;
        //code += 2;
        score += up * weightMap["year"];
        //if year are the same and month are the same, the socre up again
        if(!isTrival(feature1.month) && !isTrival(feature2.month) && feature1.month == feature2.month)
        {
            score += up * weightMap["month"];
        }
    }
    if(!isTrival(feature1.src_actor) && !isTrival(feature2.src_actor) && feature1.src_actor == feature2.src_actor)
    {
        //cerr<<"(" << feature1.src_actor << ", " << feature2.src_actor<< ") " << score << endl;
        //code += 4;
        score += up * weightMap["src_actor"];
    }
    if(!isTrival(feature1.src_agent) && !isTrival(feature2.src_agent) && feature1.src_agent == feature2.src_agent)
    {
        //cerr<<"(" << feature1.src_agent << ", " << feature2.src_agent<< ") " << score << endl;
        //code += 8;
        score += up * weightMap["src_agent"];
    }
    if(!isTrival(feature1.tgt_actor) && !isTrival(feature2.tgt_actor) && feature1.tgt_actor == feature2.tgt_actor)
    {
        //cerr<<"(" << feature1.tgt_actor << ", " << feature2.tgt_actor<< ") " << score << endl;
        //code += 16;
        score += up * weightMap["tgt_actor"];
    }
    if(!isTrival(feature1.tgt_agent) && !isTrival(feature2.tgt_agent) && feature1.tgt_agent == feature2.tgt_agent)
    {
        //cerr<<"(" << feature1.tgt_agent << ", " << feature2.tgt_agent<< ") " << score << endl;
        //code += 32;
        score += up * weightMap["tgt_agent"];
    }
    //6 here is 6 features, and 1.0 is the weight for cosine similarity
    // if(score == score_threshold)
    // {
    //     cout << "score: " << score << endl;
    //     cout << feature1.id << "," << feature2.id << endl;
    //     cout << feature1.code << "," << feature2.code << endl;
    //     cout << feature1.rootcode << "," << feature2.rootcode << endl;
    //     // cout << feature1.country_code << "," << feature2.country_code << endl;
    //     cout << feature1.year << "," << feature2.year << endl;
    //     cout << feature1.month << "," << feature2.month << endl;
    //     cout << feature1.src_actor << "," << feature2.src_actor << endl;
    //     cout << feature1.src_agent << "," << feature2.src_agent << endl;
    //     cout << feature1.tgt_actor << "," << feature2.tgt_actor << endl;
    //     cout << feature1.tgt_agent << "," << feature2.tgt_agent << endl;

    // }
    return  score;

}

//this is not for feature weight learning yet, just everybody has the same weight for each property and also the vector similarity
int getPropertyValueMatch(vector<Sentence *> &sentenceArray, vector<Incidence *> &incidenceArray, int incidenceindex, int sentenceindex, bool fromsource, int score_threshold, map<string, int> &weightMap)
{
    int pairValues = 0;

    vector<int> sentencesid = (*(incidenceArray[incidenceindex])).sentencesid;

    SentenceFeatureValue sen = (*((*(sentenceArray[sentenceindex])).featureValue));

    //if the original incidence only have 1 sentence which is the sentence to be move, then assign this value to be a random number, I think.
    if(fromsource && sentencesid.size() == 1)
    {
        //I am using 6 features now, this 5 needs to be changed based on how many features you are considering.
        // pairValues = generateRandomInteger(0, 12);
        //return pairValues;
        //pairValues=0;
        return 0;
    }
    else
    {
        for(int id : sentencesid)
        {
            if(id != sentenceindex)
            {
                SentenceFeatureValue currsen = (*((*(sentenceArray[id])).featureValue));
                pairValues += getMatchWithinSentences(sen, currsen, score_threshold, weightMap);
            }
        }
    }
    // if(pairValues == 8)
    // {
    //     cout << "incidence index: " << incidenceindex << endl;
    //     cout << "sentence index: " << sentenceindex << endl;
    // }
    return pairValues / (sentencesid.size());

}

//get pairwise feature match value
// public map<string, int>& getPairwiseFeatureMap(int sentenceindex, int destinationincidenceindex)
// {

// }

void linkSentenceToIncidence(vector<Incidence *> &incidenceArray, int destincidenceindex, int sourceincidenceindex, int sentenceindex, int SenIndexInOriginalSentenceIds, SharedResources &shared)
{
    Incidence *source = (incidenceArray[sourceincidenceindex]);
    vector<int> sourceSentencesid = (*source).sentencesid;
    //means there is no sentence to move around at all, so we should swap this incidence with the last "active" incidence.
    (*source).lock();
    sourceSentencesid.erase(sourceSentencesid.begin() + SenIndexInOriginalSentenceIds);
    (*source).unlock();

    //if the incidence become empty then move it to the end of the vector.
    if(sourceSentencesid.empty())
    {
        // cout<<"here again??"<<endl;
        shared.lock();
        incidenceArray[sourceincidenceindex] = incidenceArray[lastActiveIncidenceIndex];
        //swap the last active incidenceIndex at the empty spot, then decreas the lastActiveIncidenceIndex.
        shared.lastActiveIncidenceIndex = shared.lastActiveIncidenceIndex - 1;
        shared.unlock();
        // cout<<"after linked last active: "<<
        //cout<<lastActiveIncidenceIndex<<endl;
    }
    //need to make thsi a pointer otherwise this information add new sentence into it will not get stored!
    Incidence *dest = (incidenceArray[destincidenceindex]);
    (*dest).lock();
    (*dest).sentencesid.push_back(sentenceindex);
    (*dest).unlock();
}

void do_work(vector<Incidence *> &incidenceArray, vector<Sentence *> &sentenceArray, SharedResources &shared, int iteration, int score, int threadid)
{

    cout << "thread id: " << threadid << " get started!" << endl;
    int linkedcount = 0;
    for(int i = 0; i < iteration; i++)
    {
        try
        {
            int sizeOfIncidenceArray = incidenceArray.size();
            //cout<<"size of incidence array is: "<<sizeOfIncidenceArray<<endl;
            int sourceIncidenceIndex = generateRandomInteger(0, sizeOfIncidenceArray - 1);
            int destinationIncidenceIndex = generateRandomInteger(0, sizeOfIncidenceArray - 1);
            //if source and destination are the same thing, then do nothing.
            if(sourceIncidenceIndex == destinationIncidenceIndex)
            {
                continue;
            }
            Incidence sourceIncidence = *(incidenceArray[sourceIncidenceIndex]);
            string sourceIncidenceId = sourceIncidence.inci_id;
            int size = sourceIncidence.sentencesid.size();
            //ToFo:if there is no sentence in the incidence we need to replace the tail incidence with the current one.
            if(size == 0)
            {
                //sawp the incidence with the last one
                shared.lock();
                incidenceArray[sourceIncidenceIndex] = incidenceArray[shared.lastActiveIncidenceIndex];
                //swap the last active incidenceIndex at the empty spot, then decreas the lastActiveIncidenceIndex.

                shared.lastActiveIncidenceIndex = shared.lastActiveIncidenceIndex - 1;
                shared.unlock();
                continue;
            }
            //otherwise choose a sentence to move
            int sentenceIndexInSource = generateRandomInteger(0, size - 1);
            //in the sentencesid store the index of the global sentence array.
            int sentenceGlobalIndex = sourceIncidence.sentencesid[sentenceIndexInSource];
            //cout << to_string(sentenceIndexInSource) + " " + to_string(sentenceGlobalIndex) << endl;


            string incidenceDestination = (*(incidenceArray[ destinationIncidenceIndex])).inci_id;
            map<string, int> weightMap;

            weightMap["code"] = 1;
            weightMap["rootcode"] = 0;
            weightMap["year"] = 1;
            weightMap["month"] = 1;
            weightMap["src_actor"] = 1;
            weightMap["src_agent"] = 1;
            weightMap["tgt_actor"] = 1;
            weightMap["tgt_agent"] = 1;

            double originalSimilarity = getSentenceSimilarityWithinIncidence(sentenceArray, incidenceArray, sourceIncidenceIndex, sentenceGlobalIndex, true);
            double newSimilarity = getSentenceSimilarityWithinIncidence(sentenceArray, incidenceArray, destinationIncidenceIndex, sentenceGlobalIndex, false);
            double originalPairs = getPropertyValueMatch(sentenceArray, incidenceArray, sourceIncidenceIndex, sentenceGlobalIndex, true, score, weightMap);
            double newPairs = getPropertyValueMatch(sentenceArray, incidenceArray, destinationIncidenceIndex, sentenceGlobalIndex, false, score, weightMap);
            //using the metroplis hastings algorithms here
            //double originalFinalScore = (1.0 / 13.0) * originalSimilarity + (12.0 / 13.0) * originalPairs;
            //double newFinalScore = (1.0 / 13.0) * newSimilarity + (12.0 / 13.0) * newPairs;
            //double mh_value = min(1.0, originalFinalScore / newFinalScore);
            //double mh_value = min(1.0, newSimilarity / originalSimilarity );

            //not link if 3 pairs not match for the new configureation
            //give a new similairty threshold and asee
            if(newPairs >= score)
            {
                if(originalSimilarity < newSimilarity && newSimilarity >= 0.5)
                {
                    //cout << "new similarity: " << newSimilarity << endl;
                    linkSentenceToIncidence(incidenceArray, destinationIncidenceIndex, sourceIncidenceIndex, sentenceGlobalIndex, sentenceIndexInSource, ref(shared));
                   // cout<<"sentence globalindex: "<<sentenceGlobalIndex<<endl;
                    linkedcount++;

                    // vector<int> sentencesid = (*(incidenceArray[destinationIncidenceIndex])).sentencesid;
                    // int curr1 = -1;
                    // int curr2 = -1;


                    // for(unsigned int j = 0; j < sentencesid.size(); j++)
                    // {
                    //     int curr = sentencesid[j];
                    //     if(j == 0)
                    //     {
                    //         curr1 = curr;
                    //     }
                    //     if(j == 1)
                    //     {
                    //         curr2 = curr;
                    //     }

                    // }
                    // cout<<"global1: "<<endl;
                    // cout<<"global2: "<<endl;
                    // cout << "cosine similiarty immediate check: " << getSimilarityBySentenceId(sentenceArray, curr1, curr2) << endl;
                    //out << getSimilarityBySentenceId(sentenceArray, curr1, curr2) << endl;

                }
                // tring realid = (*((*(sentenceArray[sentenceGlobalIndex])).featureValue)).id;
                //cout <<"id when link" realid << endl;


                /*  else
                  {
                      //only when >0.3 random number less than mn_value will link, give it somechance when the dest result<0.35 to still link them
                      //if(mh_value!=1&&generateRandomInteger(96, 100) / 100.0 < mh_value)
                      if(generateRandomInteger(0, 1000) / 1000.0 < 0.002)
                      {
                          linkSentenceToIncidence(incidenceArray, destinationIncidenceIndex, sourceIncidenceIndex, sentenceGlobalIndex, sentenceIndexInSource,ref(shared));
                          linkedcount++;

                      }

                  }*/

            }

        }
        catch (...)
        {
            // catch anything thrown within try block that derives from std::exception
            cout << "what is the error???" << i << endl;
            //cout << exc.what();
        }
    }


    cout << "linked count is: " << linkedcount << endl;
}


int main(int argc, char **argv)
{
    //initialize global feature weight
    GlobalFeatureWeight globalFeatureWeight;
    //cout << globalFeatureWeight.featureWeight["code"] << endl;
    bool alive = true;
    vector<Sentence *> sentenceArray;
    // if you input two paramters the argc will be 3.
    if (argc < 4)
    {
        cout << "input the scorethreshold and also the sample number: " << endl;
        return 0;
    }
    //the score threshhold to make a decision link or not link
    int score = atoi(argv[1]);
    //iteration times
    int iteration = atoi(argv[2]);

    string outputfile = argv[3];

    cout << "score threshold is: " << score << endl;
    cout << "No of iterations: " << iteration << endl;
    while (alive)
    {
        Json::Value root;   // will contains the root value after parsing.
        Json::Reader reader;
        std::ifstream test("../dataWithAllPropertyWithEmbedding300-new.data", std::ifstream::binary);
        cout << "start to parse!" << endl;
        bool parsingSuccessful = reader.parse( test, root, false );
        cout << "end parse!" << endl;

        if ( !parsingSuccessful )
        {
            // report to the user the failure and their locations in the document.
            // std::cout  << reader.getFormatedErrorMessages()
            //      << "\n";
            std::cout << "failed to parse" << endl;
        }
        else
        {
            //root is json array! here
            //u can call root[0], root[1], root.size(); we have 546022 events here.
            //for each event create a documents
            std::cout << "successfully parsed" << endl;
            Json::FastWriter fastWriter;
            for(unsigned i = 0; i < root.size(); i++)
            {
                //string doc=fastWriter.write(root[i]["doc"]);
                string code = fastWriter.write(root[i]["code"]);
                string rootcode = fastWriter.write(root[i]["root_code"]);
                //string country_code = fastWriter.write(root[i]["country_code"]);
                string date8 = fastWriter.write(root[i]["date8"]);
                //string geoname = fastWriter.write(root[i]["geoname"]);
                string id = fastWriter.write(root[i]["id"]);
                string year = fastWriter.write(root[i]["year"]);
                string latitude = fastWriter.write(root[i]["latitude"]);
                string longitude = fastWriter.write(root[i]["longitude"]);
                string src_actor = fastWriter.write(root[i]["src_actor"]);
                string src_agent = fastWriter.write(root[i]["src_agent"]);
                string tgt_actor = fastWriter.write(root[i]["tgt_actor"]);
                string tgt_agent = fastWriter.write(root[i]["tgt_agent"]);
                string month = fastWriter.write(root[i]["month"]);
                string day = fastWriter.write(root[i]["day"]);

                //string embed=fastWriter.write(root[i]["embed"]);
                auto embed2 = root[i]["embed"];
                //this new is very important, otherwise the vector will be deallocated!!!!
                int *embed3 = new int[EMBED_SIZE];
                for(int j = 0; j < EMBED_SIZE; j++)
                {
                    embed3[j] = embed2[j].asInt();
                }

                SentenceFeatureValue *value = new SentenceFeatureValue(code, rootcode,  date8,  id, year, src_actor, src_agent, tgt_actor, tgt_agent, month, day, embed3, i);
                sentenceArray.push_back(new Sentence(id, value));


            }
        }
        alive = false;
    }

    int *vec1 = (*((*(sentenceArray[10000])).featureValue)).embed;
    int *vec2 = (*((*(sentenceArray[9999])).featureValue)).embed;
    double cosine = cosineSimilarity(vec1, vec2);
    cout << cosine << endl;

    vector<Incidence *> incidenceArray;
    vector<Subincidence *> subincidenceArray;

    int sentencesSize = sentenceArray.size();
    //initialize each sentence as an incidence.
    for(int i = 0; i < sentencesSize; i++)
    {
        vector<int> *sentencesid = new vector<int>();
        //string id = (*((*(sentenceArray[i])).featureValue)).id;
        (*sentencesid).push_back(i);
        //sentencesid is a pointer  here.
        incidenceArray.push_back(new Incidence(to_string(i), *sentencesid));
    }
    cout << "size of the incidence array is " << to_string(incidenceArray.size()) << endl;

    int  destinationIncidenceIndex = 0;
    //this will be the incidenceid.
    string incidenceDestination = "";
    int sourceIncidenceIndex = 0;
    int sizeOfIncidenceArray = 0;
    string sentenceid = "";
    string sourceIncidenceId = "";
    int globalSize = incidenceArray.size();
    lastActiveIncidenceIndex = globalSize - 1;
    SharedResources *shared = new SharedResources(globalSize - 1);

    clock_t begin = clock();
    thread t1(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 1);
    thread t2(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 2);
    thread t3(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 3);
    thread t4(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 4);
    thread t5(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 5);
    thread t6(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 6);
    thread t7(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 7);
    thread t8(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 8);

    thread t9(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 9);
    thread t10(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 10);
    thread t11(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 11);
    thread t12(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 12);
    thread t13(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 13);
    thread t14(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 14);
    thread t15(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 15);
    thread t16(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 16);

    thread t17(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 17);
    thread t18(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 18);
    thread t19(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 19);
    thread t20(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 20);
    thread t21(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 21);
    thread t22(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 22);
    thread t23(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 23);
    thread t24(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 24);


    thread t25(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 25);
    thread t26(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 26);
    thread t27(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 27);
    thread t28(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 28);
    thread t29(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 29);
    thread t30(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 30);
    thread t31(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 31);
    thread t32(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 32);

    thread t33(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 33);
    thread t34(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 34);
    thread t35(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 35);
    thread t36(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 36);
    thread t37(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 37);
    thread t38(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 38);
    thread t39(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 39);
    thread t40(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 40);

    thread t41(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 41);
    thread t42(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 42);
    thread t43(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 43);
    thread t44(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 44);
    thread t45(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 45);
    thread t46(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 46);
    thread t47(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 47);
    thread t48(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 48);

    thread t49(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 49);
    thread t50(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 50);
    thread t51(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 51);
    thread t52(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 52);
    thread t53(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 53);
    thread t54(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 54);
    thread t55(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 55);
    thread t56(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 56);


    thread t57(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 57);
    thread t58(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 58);
    thread t59(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 59);
    thread t60(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 60);
    thread t61(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 61);
    thread t62(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 62);
    thread t63(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 63);
    thread t64(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 64);

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();
    t7.join();
    t8.join();

    t9.join();
    t10.join();
    t11.join();
    t12.join();
    t13.join();
    t14.join();
    t15.join();
    t16.join();

    t17.join();
    t18.join();
    t19.join();
    t20.join();
    t21.join();
    t22.join();
    t23.join();
    t24.join();

    t25.join();
    t26.join();
    t27.join();
    t28.join();
    t29.join();
    t30.join();
    t31.join();
    t32.join();

    t33.join();
    t34.join();
    t35.join();
    t36.join();
    t37.join();
    t38.join();
    t39.join();
    t40.join();

    t41.join();
    t42.join();
    t43.join();
    t44.join();
    t45.join();
    t46.join();
    t47.join();
    t48.join();

    t49.join();
    t50.join();
    t51.join();
    t52.join();
    t53.join();
    t54.join();
    t55.join();
    t56.join();

    t57.join();
    t58.join();
    t59.join();
    t60.join();
    t61.join();
    t62.join();
    t63.join();
    t64.join();

    clock_t end = clock();

    double elapsed_secs = double(end - begin)/CLOCKS_PER_SEC;

    //do_work(incidenceArray,sentenceArray,*shared,iteration,score);
    // cout << "linked count: " + to_string(linkedcount) << endl;
    cout << "last active later: " + to_string((*shared).lastActiveIncidenceIndex) << endl;
    //int count = 0;
    ofstream out(outputfile);
    if(!out)
    {
        cout << "could not open file" << endl;
        return 0;
    }
    //out.close();
    out << "time taken in seconds with " << iteration << " score" << score << " in seconds: " << elapsed_secs << endl;
    for(int i = 0; i < (*shared).lastActiveIncidenceIndex; i++)
    {
        vector<int> sentencesid = (*(incidenceArray[i])).sentencesid;
        if(sentencesid.size() > 1)
        {
            //count++;
            // cout<<"you ever get here?"<<endl;
            int curr1 = -1;
            int curr2 = -1;
            int curr3 = -1;
            for(unsigned int j = 0; j < sentencesid.size(); j++)
            {
                int curr = sentencesid[j];
                if(j == 0)
                {
                    curr1 = curr;
                }
                if(j == 1)
                {
                    curr2 = curr;
                }
                if(j == 2)
                {
                    curr3 = curr;
                }

                string realid = (*((*(sentenceArray[curr])).featureValue)).id;
                cout << realid << endl;
                out << realid << endl;

            }
            cout << "cosine similiarty: " << getSimilarityBySentenceId(sentenceArray, curr1, curr2) << endl;
            out << getSimilarityBySentenceId(sentenceArray, curr1, curr2) << endl;
            if(curr3 != -1)
            {
                out << "more than 2 together!" << endl;
                out << getSimilarityBySentenceId(sentenceArray, curr2, curr3) << endl;
            }
            out << " " << endl;
            cout << " " << endl;
            cout << " " << endl;
            //return 0;
        }
        // if(count >= linkedcount - 2)
        // {
        //     return 0;
        // }

    }


    // cout << "test size is: " << test.size() << endl;
    // vector<int> ressult = shuffleTheIndexOfVector(10);
    // if( __cplusplus == 201103L ) std::cout << "C++11\n" ;
    // else if( __cplusplus == 19971L ) std::cout << "C++98\n" ;
    // else std::cout << "pre-standard C++\n" ;
    return 0;
}






