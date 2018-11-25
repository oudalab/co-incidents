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
#include <random>       // std::default_random_engine
#include <chrono>       // std::chrono::system_clock
//#include <uuid/uuid.h>
#include <map>
#include <cstdlib>
#include <cmath> 
//load json files of features from disk
//#include <jsoncpp/json/value.h>
//#include <jsoncpp/json/reader.h>
//#include <jsoncpp/json/writer.h>

#include <mutex>
#include <thread>
#include <pthread.h>
//this the dimenstion of the word embedding.
#define EMBED_SIZE 150
#define BOUND 30
//#define ITERATION 10000000
using namespace std;
int lastActiveIncidenceIndex = 0;
// Ctrl+Shift+Alt+Q: Quick Format.
// Ctrl+Shift+Alt+S: Selected Format.


/////////////////////
#include "IncidenceFeature.h"
#include "SentenceFeatureValue.h"
#include "GlobalFeatureWeight.h"
#include "Sentence.h"
#include "SharedResources.h"
#include "Incidence.h"
#include "Subincidence.h"
#include "SuperIncidence.h"

//////////


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

bool isTrival(string input)
{
    // iinput.erase(std::remove(input.begin(), input.end(), '\n'), input.end());
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



void linkSentenceToIncidence(vector<Incidence *> &incidenceArray, vector<Sentence *> &sentenceArray, int destincidenceindex, int sourceincidenceindex, int sentenceindex, int SenIndexInOriginalSentenceIds, SharedResources &shared)
{

    Incidence *source = (incidenceArray[sourceincidenceindex]);
    // int old_incidence_id=(*source).inci_id;
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
        //incidenceArray[lastActiveIncidenceIndex]
        incidenceArray[sourceincidenceindex] = incidenceArray[shared.lastActiveIncidenceIndex];
        //swap the last active incidenceIndex at the empty spot, then decreas the lastActiveIncidenceIndex.
        shared.lastActiveIncidenceIndex = shared.lastActiveIncidenceIndex - 1;
        shared.unlock();
        // cout<<"after linked last active: "<<
        //cout<<lastActiveIncidenceIndex<<endl
    }

    //need to make thsi a pointer otherwise this information add new sentence into it will not get stored!
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
            //cout<<"size of incidence array is: "<<sizeOfIncidenceArray<<endl;
            int sourceIncidenceIndex = generateRandomInteger(0, sizeOfIncidenceArray - 1);
            int destinationIncidenceIndex = generateRandomInteger(0, sizeOfIncidenceArray - 1);
            //int sourceIncidenceId = sourceIncidence.inci_id;
            //if source and destination are the same thing, then do nothing.
            if(sourceIncidenceIndex == destinationIncidenceIndex)
            {
                continue;
            }
            Incidence sourceIncidence = *(incidenceArray[sourceIncidenceIndex]);

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


            //int incidenceDestination = (*(incidenceArray[ destinationIncidenceIndex])).inci_id;
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
            //            double originalPairs = getPropertyValueMatch(sentenceArray, incidenceArray, sourceIncidenceIndex, sentenceGlobalIndex, true, score, weightMap);
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
                    linkSentenceToIncidence(incidenceArray, sentenceArray, destinationIncidenceIndex, sourceIncidenceIndex, sentenceGlobalIndex, sentenceIndexInSource, ref(shared));
                    // cout<<"sentence globalindex: "<<sentenceGlobalIndex<<endl;
                    linkedcount++;

                }
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
            //return ;
        }
    }
    //std::ofstream  out(statsfile);
    clock_t start1 = clock();
    double elapsed_secs = 0.0;
    int successivenochange = 0; //this is successive 30 seconds no change count
    bool flag = false;
    int oldindex = shared.lastActiveIncidenceIndex;
    /*for(int i = 0; i < iteration; i++)
    {*/
    //sicne each seconds will be logged too many times
    int previousSecond=-1;
    while(!shared.jumpout)
    {
        //make
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
               // (*out) << "seconds:" << (int)elapsed_secs << endl;
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

                    //break;
                    shared.lock();
                    shared.jumpout=true;
                    shared.unlock();
                }
                oldindex = currindex;

            }
        }

        try
        {
            //this is wrong
            //int sizeOfIncidenceArray = incidenceArray.size();
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

    if (sqlite3_open ("../../coincidenceData/events-1004.db", &db) != SQLITE_OK) {
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

    std::vector<std::thread> threads;
    if(!biased)
    {
        // Start threads
        for (int i = 0; i < 64; ++i) {
            threads.push_back(thread(do_work, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, i+1));
        }
        
        // Join threads
        for (int i = 0; i < 64; ++i) {
            threads[i].join();
        }
    }
    else
    {
        for (int i = 0; i < 64; ++i) {
            threads.push_back(thread(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, i+1,statsfile));
        }
        
        // Join threads
        for (int i = 0; i < 64; ++i) {
            threads[i].join();
        }
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


