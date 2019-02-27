#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <iostream>
#include <limits>
#include <string>
#include <errno.h>
#include <time.h>
#include <cstring>
#include "Sentence.h"
#include "Incidence.h"
#include <vector>

#define MAX(a,b) ( ((a) > (b)) ? (a) : (b) )
#define MIN(a,b) ( ((a) < (b)) ? (a) : (b) )

#define DOT std::cerr << ".";

//this the dimenstion of the word embedding.
const int EMBED_SIZE = 150;
//defines the successive no link number that makes the linking process stopped.
const int BOUND = 30;

// This only works for C
static const char *  currentTime () {
    time_t rawtime;
    struct tm * timeinfo;

    time (&rawtime);
    timeinfo = localtime (&rawtime);
    char * thetime = asctime(timeinfo);
    thetime[strlen(thetime)-1] = '\0';
    return (const char *) thetime;
}

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define DATE_STRING currentTime()

#define log_info(M) std::cerr << DATE_STRING << " [INFO] (" << __FILE__ << ":" << __LINE__ << ") | " << M  << "\n"

#define log_err(M) std::cerr << DATE_STRING << " [ERROR] (" << __FILE__ << ":" << __LINE__ << ": " << std::strerror(errno) << ") | " << M  << "\n"

#define check_mem(A) check((A), "Out of memory.")

void rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(),
                [](int ch) { 
                    return !std::isspace(ch);
                }).base(), s.end());
}

int dotProduct(int *vect_A, int *vect_B, int EMBED_SIZE)
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
    return (dotProduct(vec1, vec2,EMBED_SIZE) * 1.0) / (vectorLength(vec1) * vectorLength(vec2));
}

/*
generate a random number within a range, include min and max value 
*/
int generateRandomInteger(int min, int max)
{
    srand(std::chrono::system_clock::now().time_since_epoch().count());
    return min + (rand() % static_cast<int>(max - min + 1));
}

/*
give an integer split it into several other integers randomly and will add up to the integer
*/
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

/*
shuffle the number ={0,1,2,3.....n-1}
*/
vector<int> &shuffleTheIndexOfVector(int n)
{
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

bool isTrival(string input)
{
    rtrim(input);
    if(input == "\"\"" || input == "" || input == " "||input=="00000" || input.empty() || input.length() == 0)
    {
        return true;
    }
    return false;
}

/*
given two sentence check how many pairs property match
*/
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

/*
this is not using feature weight learning yet, just everybody has the same weight for each property and also the vector similarity
*/
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

//the stop criteria for now actually is not controller by the iteration. but keep it as a place holder here that we might use later.
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


            //can those bound be changed each time???
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
            //then find which incidence this near incidence sentence is belong to
            int destinationIncidenceIndex = (*(sentenceArray[nearSentenceId])).incidence_id;
            if(sourceIncidenceIndex == destinationIncidenceIndex)
            {
                continue;
            }

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
            //this weight controls both of the longiture and latitude.
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
            cout << "what is the error???" << endl;
        }
    }


    cout << "linked count is: " << linkedcount << endl;
    if(threadid == 1)
    {
        (*out).close();
        (*out).clear(); // clear flags
    }
}

#endif  // UTIL_HPP
