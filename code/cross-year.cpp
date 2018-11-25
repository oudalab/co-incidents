#include <iostream>
#include <fstream>
#include <iostream>
#include <sstream>
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
#include <map>
#include <cstdlib>
#include <cmath> 

#include <mutex>
#include <thread>
#include <pthread.h>
using namespace std;

#define EMBED_SIZE 150
#define BOUND 100
int lastActiveIncidenceIndex = 0;
void rtrim(std::string &);

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
    int index;
    SentenceFeatureValue()
    {

    }
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
        //doc=doc1;
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


int dotProduct(int *vect_A, int *vect_B)
{

    int product = 0;

    // Loop for calculate cot product
    for (int i = 0; i < EMBED_SIZE; i++)

        product = product + vect_A[i] * vect_B[i];
    return product;
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
    if(input == "\"\"" || input == "" || input == " "||input=="00000" || input.empty() || input.length() == 0)
    {
        return true;
    }
    return false;
}
class SharedResources
{

public:
    int lastActiveIncidenceIndex;
    int totallinked=0;
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

double vectorLength(int *vect)
{
    double length = 0.0;
    for (int i = 0; i < EMBED_SIZE; i++)
    {
        length = length + (vect[i] * 1.0) * (vect[i] * 1.0);
    }
    return sqrt(length);
}


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

class Sentence
{
public:
    string sen_id;
    //thsi is to keep track which incidence this sentence is from in order to do the time based sampling
    int incidence_id;
    SentenceFeatureValue *featureValue;
    pthread_mutex_t mutex;
    Sentence(string sentenceid, SentenceFeatureValue *featureValue1, int incidence_id1)
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


void deserializeIncidence(string str,Incidence& incidence,vector<Incidence*> &incidenceArray, vector<Sentence*> &sentenceArray)
{

    stringstream stream(str);
    int parsedindex=0;
    int sentenceIndex=sentenceArray.size();
    vector<int> sentencesid=incidence.sentencesid;
    int incidenceIndex=incidenceArray.size();
    SentenceFeatureValue* v=new SentenceFeatureValue();
    
    string word;
    string inword;
    string embedstring;
    while( getline(stream, word, ',') )
    {
      parsedindex++;
      switch(parsedindex) {
      case 1 : (*v).code=word;break;
      case 2 : (*v).rootcode=word;break;
      case 3 : 
        if(word!=" "&&word!="")
        {
          try
          {
             (*v).latitude=stod(word);
          }
          catch(exception& e)
          {
            cout<<"exception encountered!"<<endl;
          }
         
        }
        break;
      case 4 : 
        if(word!=" "&&word!="")
        {
          try
          {
            (*v).longitude=stod(word);
          }
          catch(exception& e)
          {
            cout<<"exception encountered!"<<endl;
          }
        }
        break;
      case 5 : (*v).geoname=word;break;
      case 6 : (*v).date8=word;break;
      case 7 : (*v).id=word;break;
      case 8 : (*v).year=word;break;
      case 9 : (*v).src_actor=word;break;
      case 10: (*v).src_agent=word;break;
      case 11: (*v).tgt_actor=word;break;
      case 12: (*v).tgt_agent=word;break;
      case 13: (*v).month=word;break;
      case 14: (*v).day=word;break;
      case 15: 
        if(word!=" "&&word!="")
        {
         //cout<<parsedindex<<word<<endl;
         (*v).index=stoi(word); 
        }
        break;
      case 16: 
               cout<<"you ever get here??"<<endl;
               embedstring=word;
               cout<<word<<endl;
               break;
      }
    }
    Sentence* sentence=new Sentence((*v).id,v,incidenceIndex);
    sentenceArray.push_back(sentence);
    (*v).index=sentenceIndex;
    (*v).embed=new int[EMBED_SIZE];

    int innerindex=0;
    cout<<embedstring<<endl;
    stringstream ss(embedstring);
    while(getline(ss,inword,'|'))
    {
       cout<<inword<<"||";
       (*v).embed[innerindex]=stoi(inword);
       innerindex++;
    }
    
    //each incidence has a sentences array.
    incidence.sentencesid.push_back(sentenceIndex);
    //sentenceArray.push_back(sentence);

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
   // cout<<"get here 21"<<endl;
    //cout<<"incidenceArray size:"<<incidenceArray.size()<<endl;
    //cout<<"incidence index: "<<incidenceid<<endl;
    vector<int> sentencesid = (*(incidenceArray[incidenceid])).sentencesid;
    //cout<<"get here 22"<<endl;
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
      //cout<<"get here 23"<<endl;
        for(int id : sentencesid)
        {
            if(id != sentenceindex)
            {
                sen1 = id;
                //count=count+1;
                //pairwisely calculate the similarity of the current add in stuff with the snetence already in the list and compare the similarity with some threshhold.
               // cout<<"get here 21"<<endl;
                sentenceWithIncidenceSimilarity = sentenceWithIncidenceSimilarity + getSimilarityBySentenceId(sentenceArray, sen1, sen2);
                //cout<<"get here 22"<<endl;
                // if(sentenceWithIncidenceSimilarity>=0.5)
                // {
                //  cout<<"dest incidence sen:"<<sen1<<endl;
                //  cout<<"source incidence sen:"<<sen2<<endl;
                // }
            }
        }
      //cout<<"get here 24"<<endl;
    }
   // cout<<"get here 21"<<endl;
    if(!fromsource)
    {
        return sentenceWithIncidenceSimilarity / sentencesid.size();
    }
    else
    {
        return sentenceWithIncidenceSimilarity / (sentencesid.size() - 1);
    }


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
    int timeTried=0;
    /*for(int i = 0; i < iteration; i++)
    {*/
    //sicne each seconds will be logged too many times
    int previousSecond=-1;
    while(!shared.jumpout)
    {
        //timeTried++;
       // cout<<"keep trying..."<<timeTried<<endl;
        if(threadid == 1)
        {
            elapsed_secs = double(clock() - start1) / CLOCKS_PER_SEC; //in seconds
            //logging the stats every 60 seconds
            int currSeconds=(int)elapsed_secs;
            //cout<<"current seconds: "<<currSeconds<<endl;
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
                    cout<<"ever jumped out"<<endl;
                    shared.lock();
                    shared.jumpout=true;
                    shared.unlock();
                }
                oldindex = currindex;

            }
        }

        try
        {
           // int sizeOfIncidenceArray = incidenceArray.size();
            int sizeOfIncidenceArray=(shared).lastActiveIncidenceIndex;
           // cout<<"size of incidence array: "<<shared.lastActiveIncidenceIndex<<endl;
            int sizeOfSentences = sentenceArray.size();

     
            int sourceIncidenceIndex = generateRandomInteger(0, sizeOfIncidenceArray - 1);

            Incidence sourceIncidence = *(incidenceArray[sourceIncidenceIndex]);
            int size = sourceIncidence.sentencesid.size();
            //cout<<"source incidence size:"<<size<<endl;
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

           // int leftBound = max(0, sentenceGlobalIndex - explore_range);
            //sicen it includes the bound so need to -1.
           // int rightBound = min(sentenceGlobalIndex + explore_range, sizeOfSentences - 1);
            //this is different from prvious code 
            int leftBound=0;
            int rightBound=sizeOfSentences;
            int nearSentenceId = generateRandomInteger(leftBound, rightBound-1);
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
            //this latitude consider both the latitude and longitude.
            weightMap["latitude"]=1;
            //
            //cout<<"weight map get loaded?"<<endl;
            if(sourceIncidenceIndex>=shared.lastActiveIncidenceIndex)
            {
              continue;
            }
            double originalSimilarity = getSentenceSimilarityWithinIncidence(sentenceArray, incidenceArray, sourceIncidenceIndex, sentenceGlobalIndex, true);
            //cout<<"get here 1"<<endl;
            if(destinationIncidenceIndex>=shared.lastActiveIncidenceIndex)
            {
              continue;
            }
            double newSimilarity = getSentenceSimilarityWithinIncidence(sentenceArray, incidenceArray, destinationIncidenceIndex, sentenceGlobalIndex, false);
            //            double originalPairs = getPropertyValueMatch(sentenceArray, incidenceArray, sourceIncidenceIndex, sentenceGlobalIndex, true, score, weightMap);
            //cout<<"get here 2"<<endl;
            double newPairs = getPropertyValueMatch(sentenceArray, incidenceArray, destinationIncidenceIndex, sentenceGlobalIndex, false, score, weightMap);
            //cout<<"get here 3"<<endl;
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
                    shared.lock();
                    shared.totallinked=shared.totallinked+1;
                    shared.unlock();
                    cout << "new incidence get linked linked count: " << shared.totallinked << endl;
                    
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


int main(int argc, char *argv[])
{
  string firstyear=std::string(argv[3]);
  string secondyear=std::string(argv[4]);
  ifstream in(firstyear+".rst");
  ifstream in2(secondyear+".rst");

  if(!in) {
    cout << "Cannot open input file.\n";
    return 1;
  }
  vector<Incidence*> incidenceArray;//=new vector<Incidence*>();
  vector<Sentence*> sentenceArray;//=new vector<Sentence*>();
  //char str[255];
  string data="";
  int incidenceIndex=0;
  
  //Incidence incidence;
  Incidence* incidence_pointer;
  int linenumber=0;
  while(in) {
    
    if(linenumber<=7)
    {
      getline(in, data);
      linenumber++;
      continue;
    }

    getline(in, data);
     if (data==" ")
        {
            incidenceIndex++;
            vector<int>* sentencesid=new vector<int>();
            incidenceArray.push_back(new Incidence(incidenceIndex,(*sentencesid)));
            incidence_pointer=incidenceArray[(incidenceArray).size()-1];
            continue;
        }
        else
        {
          deserializeIncidence(data,*incidence_pointer, ref(incidenceArray),ref(sentenceArray));
        }
  }
  in.close();
   //duplciate code can be extracted
  linenumber=0;
  while(in2) {
    
    if(linenumber<=7)
    {
      getline(in2, data);
      linenumber++;
      continue;
    }
      // delim defaults to '\n'
    //' ' is the character that I wrote to the file.
    getline(in2, data);
     if (data==" ")
        {
            incidenceIndex++;
            vector<int>* sentencesid=new vector<int>();
            incidenceArray.push_back(new Incidence(incidenceIndex,(*sentencesid)));
            incidence_pointer=incidenceArray[(incidenceArray).size()-1];
            continue;
        }
        else
        {
          deserializeIncidence(data,*incidence_pointer, ref(incidenceArray),ref(sentenceArray));
        }
  }
  in2.close();
  int oldIncidenceArraySize=incidenceArray.size();
  cout<<(incidenceArray).size()<<endl;
  // cout<<((sentenceArray)[10]->featureValue)->id<<endl;
  // cout<<((sentenceArray)[11]->featureValue)->id<<endl;
  // cout<<((sentenceArray)[12]->featureValue)->id<<endl;
  // for(int j=0;j<50;j++)
  // {
  //   for(int i=0;i<150;i++)
  // {
  //   cout<<((sentenceArray)[j]->featureValue)->index<<",";
  // }

  // }


/****************************/
/***end of loading all the year data now need to start linking
*****************************/
    string incidenceDestination = "";
    string sentenceid = "";
    string sourceIncidenceId = "";
    int globalSize = incidenceArray.size();
    lastActiveIncidenceIndex = globalSize - 1;
    SharedResources *shared = new SharedResources(globalSize - 1);
    clock_t begin = clock();
    int score = atoi(argv[1]);
    //well this not get used in link code or here just as a place holder here in that way I don't need to update the code
    int iteration = atoi(argv[2]);
    string statsfile = firstyear+"-"+secondyear + ".stas";
    string outputfile = firstyear+"-"+secondyear + ".rst";

    std::vector<std::thread> threads;
    for (int i = 0; i < 64; ++i) {
        threads.push_back(thread(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, i+1, statsfile));
        
    }

    for (int i = 0; i < 64; ++i) {
        threads[i].join();
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
    // if(biased)
    // {
    //     out << "I am doing biased sampling" << endl;
    // }
    cout << "last active when start: " << oldIncidenceArraySize-1<< endl;
    cout << "last active when end: " + to_string((*shared).lastActiveIncidenceIndex) << endl;
    //cout << "sqlite count is: "<<sqlitecount<<endl;
    //cout<< "startdate: "<<startdate<<endl;
    //cout<< "enddate: "<<enddate<<endl;
    
    out << "last active when start: " <<oldIncidenceArraySize-1<< endl;
    out << "last active when end: " <<to_string((*shared).lastActiveIncidenceIndex) << endl;
    //out << "sqlite count is: "<<sqlitecount<<endl;
    //out<< "startdate: "<<startdate<<endl;
    //out<< "enddate: "<<enddate<<endl;
    
    //int totallinked=oldIncidenceArraySize-(*shared).lastActiveIncidenceIndex-1;
    cout<<"total linked:"<<shared->totallinked<<endl;
    out<<"total linked:"<<shared->totallinked<<endl;
    out<<" "<<endl;
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
                out<<","<<sentencesid.size()<<endl;
            }
            out << " " << endl;
        }
    }
  return 0;
}
