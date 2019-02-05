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
#include <sqlite3.h>
#include <random>    
#include <chrono>       // std::chrono::system_clock
#include <map>
#include <cstdlib>
#include <cmath> 
#include <mutex>
#include <thread>
#include <pthread.h>
#include "Sentence.h"
#include "GlobalFeatureWeight.h"
using namespace std;

// Ctrl+Shift+Alt+Q: Quick Format.
// Ctrl+Shift+Alt+S: Selected Format.

//this the dimenstion of the word embedding.
#define EMBED_SIZE 150
//defines the successive no link number that makes the linking process stopped.
#define BOUND 30

int lastActiveIncidenceIndex = 0;

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
    }
};



class SentenceFeatureValue
{
public:
    string code;
    string rootcode;
    double latitude;
    double longitude;
    string geoname;
    string date8;
    string id;
    string year;
    string src_actor;
    string src_agent;
    string tgt_actor;
    string tgt_agent;
    string month;
    string day;
    int *embed;
    //this will be the index in the global sentence array
    int index;
    SentenceFeatureValue(string code1, string rootcode1, string date81,  string id1, string year1, string src_actor1, string src_agent1, string tgt_actor1, string tgt_agent1, string month1, string day1, int *embed1, int index1,double latitude,double longitude, string geoname)
    {
        code = code1;
        rootcode = rootcode1;
        date8 = date81;
        id = id1;
        year = year1;
        src_actor = src_actor1;
        src_agent = src_agent1;
        tgt_actor = tgt_actor1;
        tgt_agent = tgt_agent1;
        month = month1;
        day = day1;
        embed = embed1;
        index = index1;
        latitude=latitude;
        longitude=longitude;
        geoname=geoname;
        trimall();
    };
private:
    void trimall()
    {
        rtrim(code );
        rtrim(rootcode);
        rtrim(date8 );
        rtrim(id );
        rtrim(year);
        rtrim(month);
        rtrim(day);
        rtrim(src_actor );
        rtrim(src_agent );
        rtrim(tgt_actor );
        rtrim(tgt_agent );
        rtrim(geoname);
    }
};

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

class Incidence
{
public:
    int inci_id; //should be the index in the incidence array so each time need to update it when something changed.
    string sup_id;
    pthread_mutex_t mutex;
    /*will be a list of snetence id that is in the incidence*/
    vector<int> sentencesid;
    vector<string> subincidencesid;
    IncidenceFeature featureMap;
    Incidence(int incidenceid, vector<int> sentences): inci_id(incidenceid), sentencesid(move(sentences))
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
    if ((vectorLength(vec1) * vectorLength(vec2)) == 0)
    {
        return 0;
    }
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
    vector<int> sentencesid = (*(incidenceArray[incidenceid])).sentencesid;
    int sen1 = 0;
    int sen2 = sentenceindex;
    //in order for the process to start do this:
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
                //pairwisely calculate the similarity of the current add in stuff with the snetence already in the list and compare the similarity with some threshhold.
                sentenceWithIncidenceSimilarity = sentenceWithIncidenceSimilarity + getSimilarityBySentenceId(sentenceArray, sen1, sen2);
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
    rtrim(input);
    if(input == "\"\"" || input == "" || input == " "||input=="00000" || input.empty() || input.length() == 0)
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
        score += up * weightMap["code"];
    }
    
    if(!isTrival(feature1.rootcode) && !isTrival(feature2.rootcode) && feature1.rootcode == feature2.rootcode)
    {
        score += up * weightMap["rootcode"];
    }
    
    if(!isTrival(feature1.year) && !isTrival(feature2.year) && feature1.year == feature2.year)
    {
        score += up * weightMap["year"];
        //if year are the same and month are the same, the socre up again
        if(!isTrival(feature1.month) && !isTrival(feature2.month) && feature1.month == feature2.month)
        {
            score += up * weightMap["month"];
        }
    }
    if(!isTrival(feature1.src_actor) && !isTrival(feature2.src_actor) && feature1.src_actor == feature2.src_actor)
    {
        score += up * weightMap["src_actor"];
    }
    if(!isTrival(feature1.src_agent) && !isTrival(feature2.src_agent) && feature1.src_agent == feature2.src_agent)
    {
        score += up * weightMap["src_agent"];
    }
    if(!isTrival(feature1.tgt_actor) && !isTrival(feature2.tgt_actor) && feature1.tgt_actor == feature2.tgt_actor)
    {
        score += up * weightMap["tgt_actor"];
    }
    if(!isTrival(feature1.tgt_agent) && !isTrival(feature2.tgt_agent) && feature1.tgt_agent == feature2.tgt_agent)
    {
        score += up * weightMap["tgt_agent"];
    }
    //added for geolocation
    if(!isTrival(feature1.geoname) && !isTrival(feature2.geoname) && feature1.geoname == feature2.geoname)
    {
        score += up * weightMap["geoname"];
    }
    //latitude and longitude
    if(feature1.latitude!=0 && feature2.latitude!=0 && feature1.longitude!=0 &&feature2.longitude!=0)
    {
        double lat1=feature1.latitude;
        double lat2=feature2.latitude;
        double lon1=feature1.longitude;
        double lon2=feature2.longitude;
        if(abs(lat1-lat2)<=1 && abs(lon1-lon2)<1)
        {
             score += up * weightMap["latitude"];
        }
    }
    return  score;
}

//this is not using feature weight learning yet, just everybody has the same weight for each property and also the vector similarity
int getPropertyValueMatch(vector<Sentence *> &sentenceArray, vector<Incidence *> &incidenceArray, int incidenceindex, int sentenceindex, bool fromsource, int score_threshold, map<string, int> &weightMap)
{
    int pairValues = 0;

    vector<int> sentencesid = (*(incidenceArray[incidenceindex])).sentencesid;

    SentenceFeatureValue sen = (*((*(sentenceArray[sentenceindex])).featureValue));

    //if the original incidence only have 1 sentence which is the sentence to be move, then assign this value to be a random number, I think.
    if(fromsource && sentencesid.size() == 1)
    {
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
    return pairValues / (sentencesid.size());
}

void linkSentenceToIncidence(vector<Incidence *> &incidenceArray, vector<Sentence *> &sentenceArray, int destincidenceindex, int sourceincidenceindex, int sentenceindex, int SenIndexInOriginalSentenceIds, SharedResources &shared)
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
        ///update the swapped incidence id
        Incidence *swappedIncidence = incidenceArray[shared.lastActiveIncidenceIndex];
        (*swappedIncidence).lock();
        (*swappedIncidence).inci_id = sourceincidenceindex;
        (*swappedIncidence).unlock();

        shared.lock();
        incidenceArray[sourceincidenceindex] = incidenceArray[shared.lastActiveIncidenceIndex];
        //swap the last active incidenceIndex at the empty spot, then decreas the lastActiveIncidenceIndex.
        shared.lastActiveIncidenceIndex = shared.lastActiveIncidenceIndex - 1;
        shared.unlock();
    }

    //need to make this a pointer otherwise this information add new sentence into it will not get stored!
    Incidence *dest = (incidenceArray[destincidenceindex]);
    (*dest).lock();
    (*dest).sentencesid.push_back(sentenceindex);
    (*dest).unlock();

    //update teh sentence incidence id
    Sentence *sen = sentenceArray[sentenceindex];
    (*sen).lock();
    (*sen).incidence_id = destincidenceindex;
    (*sen).unlock();
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
            int sourceIncidenceIndex = generateRandomInteger(0, sizeOfIncidenceArray - 1);
            int destinationIncidenceIndex = generateRandomInteger(0, sizeOfIncidenceArray - 1);
            //if source and destination are the same thing, then do nothing.
            if(sourceIncidenceIndex == destinationIncidenceIndex)
            {
                continue;
            }
            Incidence sourceIncidence = *(incidenceArray[sourceIncidenceIndex]);

            int size = sourceIncidence.sentencesid.size();
            //ToDo:if there is no sentence in the incidence we need to replace the tail incidence with the current one.
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
            double newPairs = getPropertyValueMatch(sentenceArray, incidenceArray, destinationIncidenceIndex, sentenceGlobalIndex, false, score, weightMap);
            
            //using the metroplis hastings algorithms here
            //double originalFinalScore = (1.0 / 13.0) * originalSimilarity + (12.0 / 13.0) * originalPairs;
            //double newFinalScore = (1.0 / 13.0) * newSimilarity + (12.0 / 13.0) * newPairs;
            //double mh_value = min(1.0, originalFinalScore / newFinalScore);
            //double mh_value = min(1.0, newSimilarity / originalSimilarity );
            
            //give a new similairty threshold and see
            if(newPairs >= score)
            {
                if(originalSimilarity < newSimilarity && newSimilarity >= 0.5)
                {
                    linkSentenceToIncidence(incidenceArray, sentenceArray, destinationIncidenceIndex, sourceIncidenceIndex, sentenceGlobalIndex, sentenceIndexInSource, ref(shared));
                    linkedcount++;
                }
            }

        }
        catch (...)
        {
            // catch anything thrown within try block that derives from std::exception
            cout << "what is the error???" << i << endl;
        }
    }
    cout << "linked count is: " << linkedcount << endl;
}

void do_work_biased(vector<Incidence *> &incidenceArray, vector<Sentence *> &sentenceArray, SharedResources &shared, int iteration, int score, int threadid,string stasfilename)
{

    cout << "thread id: " << threadid << " get started!" << endl;
    int linkedcount = 0;
    std::ofstream *out;
    if(threadid == 1)
    {
        out = new ofstream(stasfilename);
        if(!(*out))
        {
            cout << "could not open file" << endl;
        }
    }

    clock_t start1 = clock();
    double elapsed_secs = 0.0;
    int successivenochange = 0; //this is successive 30 seconds no change count
    bool flag = false;
    int oldindex = shared.lastActiveIncidenceIndex;

    int previousSecond=-1;
    while(!shared.jumpout)
    {
        if(threadid == 1)
        {
            elapsed_secs = double(clock() - start1) / CLOCKS_PER_SEC; //in seconds
            //logging the stats every 60 seconds
            int currSeconds=(int)elapsed_secs;
            if(currSeconds% 30 == 0&&(currSeconds!=previousSecond))
            {
            	previousSecond=currSeconds;
                int currindex = shared.lastActiveIncidenceIndex;
                int diff = oldindex - currindex;
                cout << "diff:" << diff << endl;
                (*out) << diff << ",";
                if(diff == 0)
                {
                    if(flag == true)
                    {
                        successivenochange++;
                    }
                    flag = true;
                }
                else
                {
                    flag = false;
                    successivenochange = 0;
                }
                oldindex = currindex;
                //10 mins explore if nothing changed the probably we converged!
                //20*30/60=10 mins
                if(successivenochange == BOUND) 
                {
                    shared.lock();
                    shared.jumpout=true;
                    shared.unlock();
                }
                oldindex = currindex;
            }
        }

        try
        {
            int sizeOfIncidenceArray=(shared).lastActiveIncidenceIndex;
            int sizeOfSentences = sentenceArray.size();


            int sourceIncidenceIndex = generateRandomInteger(0, sizeOfIncidenceArray - 1);
            Incidence sourceIncidence = *(incidenceArray[sourceIncidenceIndex]);
            int size = sourceIncidence.sentencesid.size();
            int sentenceIndexInSource = generateRandomInteger(0, size - 1);
            //in the sentencesid store the index of the global sentence array.
            int sentenceGlobalIndex = sourceIncidence.sentencesid[sentenceIndexInSource];


            // while can those bound be changed each time???
            //will be 70 percent get
            //10% ith large range 100, 90% with small range which is 80%
            int explore_range = 30;
            if(generateRandomInteger(0, 100) / 100.0 < 0.1)
            {
                explore_range = 100;
            }

            int leftBound = max(0, sentenceGlobalIndex - explore_range);
            //sicen it includes the bound so need to -1.
            int rightBound = min(sentenceGlobalIndex + explore_range, sizeOfSentences - 1);
            int nearSentenceId = generateRandomInteger(leftBound, rightBound);
            // cout<<"nearsentenceid: "<<nearSentenceId<<endl;
            //then find which incidence this near incidence sentence is belong to
            int destinationIncidenceIndex = (*(sentenceArray[nearSentenceId])).incidence_id;
            //cout<<"destinationidex "<<destinationIncidenceIndex<<endl;
            //            int sourceIncidenceId = sourceIncidence.inci_id;
            if(sourceIncidenceIndex == destinationIncidenceIndex)
            {
                continue;
            }

            // string sourceIncidenceId = sourceIncidence.inci_id;

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

            //string incidenceDestination = (*(incidenceArray[ destinationIncidenceIndex])).inci_id;
            map<string, int> weightMap;

            weightMap["code"] = 1;
            weightMap["rootcode"] = 0;
            weightMap["year"] = 1;
            weightMap["month"] = 1;
            weightMap["src_actor"] = 1;
            weightMap["src_agent"] = 1;
            weightMap["tgt_actor"] = 1;
            weightMap["tgt_agent"] = 1;
            weightMap["geoname"]=1;
            //thsi weight consider both of the longiture and latitude.
            weightMap["latitude"]=1;

            if(sourceIncidenceIndex>=shared.lastActiveIncidenceIndex)
            {
                continue;
            }
            double originalSimilarity = getSentenceSimilarityWithinIncidence(sentenceArray, incidenceArray, sourceIncidenceIndex, sentenceGlobalIndex, true);
            if(destinationIncidenceIndex>=shared.lastActiveIncidenceIndex)
            {
                continue;
            }
            double newSimilarity = getSentenceSimilarityWithinIncidence(sentenceArray, incidenceArray, destinationIncidenceIndex, sentenceGlobalIndex, false);
            //            double originalPairs = getPropertyValueMatch(sentenceArray, incidenceArray, sourceIncidenceIndex, sentenceGlobalIndex, true, score, weightMap);
            double newPairs = getPropertyValueMatch(sentenceArray, incidenceArray, destinationIncidenceIndex, sentenceGlobalIndex, false, score, weightMap);
            //using the metroplis hastings algorithms here
            //double originalFinalScore = (1.0 / 13.0) * originalSimilarity + (12.0 / 13.0) * originalPairs;
            //double newFinalScore = (1.0 / 13.0) * newSimilarity + (12.0 / 13.0) * newPairs;
            //double mh_value = min(1.0, originalFinalScore / newFinalScore);
            //double mh_value = min(1.0, newSimilarity / originalSimilarity );

            //not link if 3 pairs not match for the new configureation
            //give a new similairty threshold and asee
            //  cout<<"you ever get here?"<<endl;
            if(newPairs >= score)
            {
                if(originalSimilarity < newSimilarity && newSimilarity >= 0.5)
                {
                    //cout << "new similarity: " << newSimilarity << endl;

                    linkSentenceToIncidence(incidenceArray, sentenceArray, destinationIncidenceIndex, sourceIncidenceIndex, sentenceGlobalIndex, sentenceIndexInSource, ref(shared));
                    // cout<<"sentence globalindex: "<<sentenceGlobalIndex<<endl;
                    linkedcount++;
                   // cout << "linked count: " << linkedcount << endl;

                }
            }

        }
        catch (...)
        {
            // catch anything thrown within try block that derives from std::exception
            cout << "what is the error???" << endl;
            //cout << exc.what();
        }
    }


    cout << "linked count is: " << linkedcount << endl;
    if(threadid == 1)
    {
        (*out).close();
        (*out).clear(); // clear flags

    }

}


int main(int argc, char **argv)
{
    //initialize global feature weight
    GlobalFeatureWeight globalFeatureWeight;
    //cout << globalFeatureWeight.featureWeight["code"] << endl;
    bool alive = true;
    vector<Sentence *> sentenceArray;
    // if you input two paramters the argc will be 3.
    if (argc < 7)
     {
         cout << "input the scorethreshold and also the sample number, filename, biased or not, and startdate, and enddate: " << endl;
         return 0;
     }
    //the score threshhold to make a decision link or not link
    int score = atoi(argv[1]);
    //int score=12;
    //iteration times
    int iteration = atoi(argv[2]);
    //int iteration=1000;

    string outputfile = std::string(argv[3]) + ".rst";
    //string outputfile = "debug.rst";

    bool biased = false;
    string statsfile = std::string(argv[3]) + ".stas";
    //string statsfile = "debug.stas";
    string clibias = argv[4];
    //string clibias="1";
    //string startdate="20040102";
    //string enddate="20050606";
    //
    string startdate=argv[5];
    string enddate=argv[6];

    if(clibias == "1")
    {
        biased = true;
        cout << "you are running it with biased sampling." << endl;
    }

    cout << "score threshold is: " << score << endl;
    cout << "No of iterations: " << iteration << endl;
    int sqlitecount=0;
    /******start to connect to database********/

    char                 q[999];
    sqlite3*             db;
    sqlite3_stmt*        stmt;
    int                  row = 0;
    int                  bytes;

    string embed="";
    double latitude;
    double longitude;
    string geoname="";
    string tgt_actor="";
    string src_actor="";
    string mediasource2="";
    string target="";
    string goldstein="";
    string tgt_other_agent="";
    string code="";
    string day="";
    string month="";
    string quad_class="";
    string mediasource1="";
    string src_other_agent="";
    string id="";
    string tgt_agent="";
    string date8="";
    string year="";
    string root_code="";
    string src_agent="";

    /*******start to connect to database *******/
    //pair<int, int> adjs[4] = {make_pair(, current_node.second), ...};

    q[sizeof q - 1] = '\0';
    snprintf(
        q,
        sizeof q - 1,
        ("SELECT * FROM events WHERE date8 >='"+startdate+"' and date8<='"+enddate+"'").c_str()
        // ,
        // ""
    );

    if (sqlite3_open ("../../coincidenceData/events1114.db", &db) != SQLITE_OK) {
        fprintf(stderr, "Error opening database.\n");
        return 2;
    }
    printf("Query: %s\n", q);

    sqlite3_prepare(db, q, sizeof q, &stmt, NULL);

    bool done = false;
    //int i=-1;
    while (!done) {
        //printf("In select while\n");
        switch (sqlite3_step (stmt)) {
        case SQLITE_ROW:
        {
        	 // i=i+1;
            try
            {
                 bytes = sqlite3_column_bytes(stmt, 0);
            //printf ("count %d:,(%d bytes)\n", row,bytes);
            //this can be column 1,2 , 3 ....
            embed  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)); //embed
            latitude  = sqlite3_column_double(stmt, 1); //embed
            longitude = sqlite3_column_double(stmt, 2); //embed

            // printf ("%f \n", latitude);
            // printf ("%f \n", longitude);
             // if(row==85)
             //     {
             //        printf("I am at row 84 and ready to fail!");
             //     }
            //this is to check if the field is null or not.
            if(sqlite3_column_text(stmt, 3))
            {
                geoname  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)); //embed
            }
            if(sqlite3_column_text(stmt, 4))
            {
                tgt_actor  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)); //tgt_actor
            }
            if(sqlite3_column_text(stmt, 5))
            {
                root_code  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)); //root_code   
            }
            if(sqlite3_column_text(stmt, 6))
            {
                src_actor  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)); //src_actor
            }
             if(sqlite3_column_text(stmt, 7))
            { 
                mediasource2  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7)); //mediasource2       
            }
            if(sqlite3_column_text(stmt, 8))
            {
                target  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8)); 
            }
             if(sqlite3_column_text(stmt, 9))
            {
                goldstein  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9)); //goldstein
            }
            if(sqlite3_column_text(stmt, 10))
            {
                tgt_other_agent  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10)); //tgt_other_agent
            }
            if(sqlite3_column_text(stmt, 11))
            {
                code = reinterpret_cast<const char*>(sqlite3_column_text(stmt,11)); //code
            }
            if(sqlite3_column_text(stmt, 12))
            {
                day = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12)); //day
            }
             if(sqlite3_column_text(stmt, 13))
            {
                month = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13)); //month
            }
            if(sqlite3_column_text(stmt, 14))
            {
                quad_class = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 14)); //quad_class
            }
            if(sqlite3_column_text(stmt, 15))
            {
                mediasource1 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 15)); //mediasource1
            }
            if(sqlite3_column_text(stmt, 16))
            {
             src_other_agent= reinterpret_cast<const char*>(sqlite3_column_text(stmt, 16)); //src_other_agent   
            }
             if(sqlite3_column_text(stmt, 17))
            {
             id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 17)); //id
            }
            if(sqlite3_column_text(stmt, 18))
            {
                tgt_agent = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 18)); //tgt_agent
            }
             if(sqlite3_column_text(stmt, 19))
            {
                date8 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 19)); //date8
            }
            if(sqlite3_column_text(stmt, 20))
            {
                year = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 20)); //year
            }
            if(sqlite3_column_text(stmt, 21))
            {
               src_agent=reinterpret_cast<const char*>(sqlite3_column_text(stmt, 21)); //source agent
            }
         //int embedsize=150; //since the last position does not have the ,so the length that has the number will be (x+1)/2                 
         int *embed3 = new int[EMBED_SIZE];
         for(int j = 0; j < EMBED_SIZE; j++)
         {
             //out<<embed.at(j*2+1)<<"*";
             embed3[j] = (int)embed.at(j*2+1);
            // cout<<embed3[j]<<"|";
         }

        
         SentenceFeatureValue *value = new SentenceFeatureValue(code, root_code, date8, id, year, src_actor, src_other_agent,tgt_actor,tgt_agent, month, day, embed3, row,latitude,longitude,geoname);
         //printf("did you ever get here?");
        
         //i will be the incidence id for this sentence
         sentenceArray.push_back(new Sentence(id, value, row));
           sqlitecount=row;
           if(row>0&&row<100)
           {
            printf ("count %d:,(%d bytes)\n", row,bytes);
           }  
            
            row++;
            break;

            }
            catch (exception& e)
          {
            cout <<"my own message"<< e.what() << '\n';
          }

        }
          

        case SQLITE_DONE:
        {
        	done = true;
            break;
        }
            
        default:
        {
        	fprintf(stderr, "Failed.\n");
            return 1;
        }
            
        }
    }

    sqlite3_finalize(stmt); 
    cout<<"finished loading data from sqlite database!"<<endl;

    int *vec1 = (*((*(sentenceArray[10])).featureValue)).embed;
    int *vec2 = (*((*(sentenceArray[9])).featureValue)).embed;
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
        incidenceArray.push_back(new Incidence(i, *sentencesid));
    }
    cout << "size of the incidence array is " << to_string(incidenceArray.size()) << endl;


    string incidenceDestination = "";
    string sentenceid = "";
    string sourceIncidenceId = "";
    int globalSize = incidenceArray.size();
    lastActiveIncidenceIndex = globalSize - 1;
    SharedResources *shared = new SharedResources(globalSize - 1);

    clock_t begin = clock();
    if(!biased)
    {
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
    }
    else
    {
        thread t1(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 1,statsfile);
        thread t2(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 2,statsfile);
        thread t3(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 3,statsfile);
        thread t4(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 4,statsfile);
        thread t5(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 5,statsfile);
        thread t6(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 6,statsfile);
        thread t7(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 7,statsfile);
        thread t8(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 8,statsfile);

        thread t9(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 9,statsfile);
        thread t10(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 10,statsfile);
        thread t11(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 11,statsfile);
        thread t12(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 12,statsfile);
        thread t13(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 13,statsfile);
        thread t14(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 14,statsfile);
        thread t15(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 15,statsfile);
        thread t16(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 16,statsfile);

        thread t17(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 17,statsfile);
        thread t18(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 18,statsfile);
        thread t19(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 19,statsfile);
        thread t20(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 20,statsfile);
        thread t21(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 21,statsfile);
        thread t22(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 22,statsfile);
        thread t23(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 23,statsfile);
        thread t24(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 24,statsfile);


        thread t25(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 25,statsfile);
        thread t26(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 26,statsfile);
        thread t27(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 27,statsfile);
        thread t28(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 28,statsfile);
        thread t29(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 29,statsfile);
        thread t30(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 30,statsfile);
        thread t31(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 31,statsfile);
        thread t32(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 32,statsfile);

        thread t33(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 33,statsfile);
        thread t34(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 34,statsfile);
        thread t35(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 35,statsfile);
        thread t36(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 36,statsfile);
        thread t37(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 37,statsfile);
        thread t38(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 38,statsfile);
        thread t39(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 39,statsfile);
        thread t40(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 40,statsfile);

        thread t41(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 41,statsfile);
        thread t42(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 42,statsfile);
        thread t43(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 43,statsfile);
        thread t44(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 44,statsfile);
        thread t45(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 45,statsfile);
        thread t46(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 46,statsfile);
        thread t47(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 47,statsfile);
        thread t48(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 48,statsfile);

        thread t49(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 49,statsfile);
        thread t50(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 50,statsfile);
        thread t51(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 51,statsfile);
        thread t52(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 52,statsfile);
        thread t53(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 53,statsfile);
        thread t54(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 54,statsfile);
        thread t55(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 55,statsfile);
        thread t56(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 56,statsfile);


        thread t57(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 57,statsfile);
        thread t58(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 58,statsfile);
        thread t59(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 59,statsfile);
        thread t60(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 60,statsfile);
        thread t61(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 61,statsfile);
        thread t62(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 62,statsfile);
        thread t63(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 63,statsfile);
        thread t64(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, 64,statsfile);


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


    }
    clock_t end = clock();

    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

    //do_work_biased(incidenceArray,sentenceArray,*shared,iteration,score);
    // cout << "linked count: " + to_string(linkedcount) << endl;
  
    //int count = 0;
    ofstream out(outputfile);
    // out.open(outputfile);
    if(!out)
    {
        cout << "could not open file" << endl;
        return 0;
    }
    //out.close();
    out << "time taken in seconds with " << iteration << " score" << score << " in seconds: " << elapsed_secs << endl;
    if(biased)
    {
        out << "I am doing biased sampling" << endl;
    }
    cout << "last active when start: " << sentenceArray.size() << endl;
    cout << "last active when end: " + to_string((*shared).lastActiveIncidenceIndex) << endl;
    cout << "sqlite count is: "<<sqlitecount<<endl;
    cout<< "startdate: "<<startdate<<endl;
    cout<< "enddate: "<<enddate<<endl;
    
    out << "last active when start: " << sentenceArray.size() << endl;
    out << "last active when end: " <<(*shared).lastActiveIncidenceIndex << endl;
    out << "sqlite count is: "<<sqlitecount<<endl;
    out<< "startdate: "<<startdate<<endl;
    out<< "enddate: "<<enddate<<endl;
    
    int totallinked=sentenceArray.size()-(*shared).lastActiveIncidenceIndex;
    cout<<"total linked:"<<totallinked<<endl;
    out<<"total linked:"<<totallinked<<endl;
    out<<" "<<endl;
    for(int i = 0; i < (*shared).lastActiveIncidenceIndex; i++)
    {
        vector<int> sentencesid = (*(incidenceArray[i])).sentencesid;
        int sentencesidsize=sentencesid.size();
        if(sentencesidsize>=1)
        {
            //count++;
            // cout<<"you ever get here?"<<endl;
            int curr1 = -1;
            int curr2 = -1;
            int curr3 = -1;
            int prev=sentencesid[0];

            for(unsigned int j = 0; j < sentencesid.size(); j++)
            {
                int curr = sentencesid[j];
                SentenceFeatureValue v=(*((*(sentenceArray[curr])).featureValue));

                out<<v.code<<","<<v.rootcode<<","<<v.latitude<<","<<v.longitude<<","<<v.geoname<<","
                <<v.date8<<","<<v.id<<","<<v.year<<","<<
                v.src_actor<<","<<v.src_agent<<","<<v.tgt_actor<<","<<v.tgt_agent<<","<<v.month<<","<<v.day<<","<<v.index<<",";
                for(int k=0;k<EMBED_SIZE;k++)
                {
                    out<<(v.embed)[k];
                    if(k!=EMBED_SIZE-1)
                    {
                        out<<"|";
                    }
                }
                out<<sentencesidsize<<","<<endl;

                //string realid = (*((*(sentenceArray[curr])).featureValue)).id;
                //cout << realid << endl;
                //out << realid << endl;
                // if(j!=0)
                // {
                // //current sentence similarity with the previous one
                //  out << getSimilarityBySentenceId(sentenceArray, prev, curr) << endl;
                // }
                //set the prev value
               // prev=curr;
            }
            //cout << "cosine similiarty: " << getSimilarityBySentenceId(sentenceArray, curr1, curr2) << endl;
            //out << getSimilarityBySentenceId(sentenceArray, curr1, curr2) << endl;
            // if(curr3 != -1)
            // {
            //     out << "more than 2 together!" << endl;
            //     out << getSimilarityBySentenceId(sentenceArray, curr2, curr3) << endl;
            // }

            //this means we jump to another incidence.
            out << " " << endl;
           // cout << " " << endl;
            //cout << " " << endl;
            //return 0;
        }
    }


    // cout << "test size is: " << test.size() << endl;
    // vector<int> ressult = shuffleTheIndexOfVector(10);
    // if( __cplusplus == 201103L ) std::cout << "C++11\n" ;
    // else if( __cplusplus == 19971L ) std::cout << "C++98\n" ;
    // else std::cout << "pre-standard C++\n" ;
    return 0;
}








