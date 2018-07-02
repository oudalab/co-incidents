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
    SentenceFeatureValue(string code1, string date81,  string id1, string year1, string src_actor1, string src_agent1, string tgt_actor1, string tgt_agent1, string month1, string day1, int *embed1, int index1)
    {
        code = code1;
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
    Sentence(string sentenceid, SentenceFeatureValue *featureValue1)
    {
        sen_id = sentenceid;
        featureValue = featureValue1;
    }
};

class Incidence
{
public:
    string inci_id;
    string sup_id;
    /*will be a list of snetence id that is in the incidence*/
    vector<int> sentencesid;
    vector<string> subincidencesid;
    IncidenceFeature featureMap;
    Incidence(string incidenceid, vector<int> sentences): inci_id(incidenceid), sentencesid(move(sentences)) {};
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
/**** ok, the goal of the simulatio for now is to cluster all the sentences that are similar to each otehr say who has the similarity bigger than .5*****/

// double *loadMatrix()
// {
//     int count = 0;

//     double similarity [length];
//     string line;

//     cout << "Testing loading of file." << endl;
//     ifstream myfile ("randommatrix");
//     if ( myfile.is_open() )
//     {
//         while ( ! myfile.eof() )
//         {
//             getline (myfile, line);
//             //cout<<"line: "+line<<endl;
//             //logs.at(count) = line;
//             try
//             {
//                 similarity[count] = stod(line);
//             }
//             catch(...)
//             {
//                 //cout<<"the count is :"<<count<<endl<<endl<<endl;
//                 return similarity;
//             }
//             //cout<<"array: "<<similarity[count]<<endl;
//             count++;
//         }
//         myfile.close();
//     }
//     else
//     {
//         cout << "Unable to open file." << endl;
//     }
//     /*still return the object on the stack*/
//     return similarity;
// }

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

double getSimilarityByMatrixIndex(double *matrix, int row, int col)
{
    if(row == col)
        return 1.0;
    else if(row > col)
    {
        //return matrix[row*xlength+col];
        // row is 0 then add 1
        // row is 1 add in 1 and 2
        // row is 2 then add in 1,2 and 3
        int index = (1 + row) * row / 2 + col;
        return matrix[index];

    }
    else
    {
        //since it is symmetric
        return getSimilarityByMatrixIndex(matrix, col, row);
    }

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

// void generateGuidString(string guidStr)
// {
//      _TUCHAR *guidStr = 0x00;

//      GUID *pguid = 0x00;

//      pguid = new GUID;

//      CoCreateGuid(pguid);

//      // Convert the GUID to a string
//      UuidToString(pguid, &guidStr);

//      delete pguid;

// }


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

double getSentenceSimilarityWithinIncidence(vector<Sentence *> &sentenceArray, vector<Incidence *> &incidenceArray, int incidenceid, int sentenceindex)
{
    double sentenceWithIncidenceSimilarity = 0;
    // double similarityInOldIncidence = 0;
    vector<int> sentencesid = (*(incidenceArray[incidenceid])).sentencesid;
    //int count=0;
    int sen1 = 0;
    int sen2 = sentenceindex;
    //in order for the proces to start do this:
    if(sentencesid.size() == 1)
    {
        sentenceWithIncidenceSimilarity = generateRandomInteger(0, 100) / 100.0;
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
            }
        }
    }
    return sentenceWithIncidenceSimilarity;

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
int getMatchWithinSentences(SentenceFeatureValue &feature1, SentenceFeatureValue &feature2, int score_threshold)
{
    int score = 0;
    int up = 2;
    if(!isTrival(feature1.code) && !isTrival(feature2.code) && feature1.code == feature2.code)
    {
        //cout<<"faatyre start"<<endl;
        //cerr<<"(" << feature1.code << ", " << feature2.code<< ") " << score << endl;
        score += up;
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
        score += up;
        //if year are the same and month are the same, the socre up again
        if(!isTrival(feature1.month) && !isTrival(feature2.month) && feature1.month == feature2.month)
        {
            score += up;
        }
    }
    if(!isTrival(feature1.src_actor) && !isTrival(feature2.src_actor) && feature1.src_actor == feature2.src_actor)
    {
        //cerr<<"(" << feature1.src_actor << ", " << feature2.src_actor<< ") " << score << endl;
        //code += 4;
        score += up;
    }
    if(!isTrival(feature1.src_agent) && !isTrival(feature2.src_agent) && feature1.src_agent == feature2.src_agent)
    {
        //cerr<<"(" << feature1.src_agent << ", " << feature2.src_agent<< ") " << score << endl;
        //code += 8;
        score += up;
    }
    if(!isTrival(feature1.tgt_actor) && !isTrival(feature2.tgt_actor) && feature1.tgt_actor == feature2.tgt_actor)
    {
        //cerr<<"(" << feature1.tgt_actor << ", " << feature2.tgt_actor<< ") " << score << endl;
        //code += 16;
        score += up;
    }
    if(!isTrival(feature1.tgt_agent) && !isTrival(feature2.tgt_agent) && feature1.tgt_agent == feature2.tgt_agent)
    {
        //cerr<<"(" << feature1.tgt_agent << ", " << feature2.tgt_agent<< ") " << score << endl;
        //code += 32;
        score += up;
    }
    //6 here is 6 features, and 1.0 is the weight for cosine similarity
    if(score == score_threshold)
    {
        cout << "score: " << score << endl;
        cout << feature1.id << "," << feature2.id << endl;
        cout << feature1.code << "," << feature2.code << endl;
       // cout << feature1.country_code << "," << feature2.country_code << endl;
        cout << feature1.year<< "," << feature2.year << endl;
        cout << feature1.month << "," << feature2.month<< endl;
        cout << feature1.src_actor << "," << feature2.src_actor << endl;
        cout << feature1.src_agent << "," << feature2.src_agent << endl;
        cout << feature1.tgt_actor << "," << feature2.tgt_actor << endl;
        cout << feature1.tgt_agent << "," << feature2.tgt_agent << endl;

    }
    return  score;

}

//this is not for feature weight learning yet, just everybody has the same weight for each property and also the vector similarity
int getPropertyValueMatch(vector<Sentence *> &sentenceArray, vector<Incidence *> &incidenceArray, int incidenceindex, int sentenceindex, bool fromsource, int score_threshold)
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
                pairValues += getMatchWithinSentences(sen, currsen,score_threshold);
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

void linkSentenceToIncidence(vector<Incidence *> &incidenceArray, int destincidenceindex, int sourceincidenceindex, int sentenceindex, int SenIndexInOriginalSentenceIds)
{
    vector<int> sourceSentencesid = (*(incidenceArray[sourceincidenceindex])).sentencesid;
    //means there is no sentence to move around at all, so we should swap this incidence with the last "active" incidence.
    sourceSentencesid.erase(sourceSentencesid.begin() + SenIndexInOriginalSentenceIds);

    //if the incidence become empty then move it to the end of the vector.
    if(sourceSentencesid.empty())
    {
        // cout<<"here again??"<<endl;
        incidenceArray[sourceincidenceindex] = incidenceArray[lastActiveIncidenceIndex];
        //swap the last active incidenceIndex at the empty spot, then decreas the lastActiveIncidenceIndex.
        lastActiveIncidenceIndex = lastActiveIncidenceIndex - 1;
        //cout<<lastActiveIncidenceIndex<<endl;
    }

    (*(incidenceArray[destincidenceindex])).sentencesid.push_back(sentenceindex);
}


// int main()

// {

//       time_t now = time(0);

//         // convert now to string form
//       char* dt = ctime(&now);
//      // srand(time(0));

//       cout << "The local date and time is: " << dt << endl;
//       //loadMatrix();
//      /*load the simulated probability matrxi.*/
//      double* matrix=loadMatrix();


//      int i=0;
//      //Sentence* sentenceArray[xlength];,
//      vector<Sentence*> sentenceArray;
//      vector<Incidence*> incidenceArray;
//      vector<Subincidence*> subincidenceArray;
//      //Incidence* incidenceArray[xlength];

//      for(int i=0;i<xlength;i++)
//      {
//         sentenceArray.push_back(new Sentence(to_string(i)));
//      }
//      //initialize all the incidence.
//      for(int i=0;i<xlength;i++)
//      {
//       //this will allocate on the stack.
//              vector<string> sentencesid;
//              sentencesid.push_back(to_string(i));
//              incidenceArray.push_back(new Incidence(to_string(i),sentencesid));
//      }

//    //  cout<<"sentenceid in the incidence: "<<(*(incidenceArray[11])).sentencesid[0]<<endl;
//      int sentenceToMove=0;
//      int incidenceDestinationIndex=0;
//      //this will be the incidenceid.
//      string incidenceDestination="";
//      int sourceIncidenceIndex=0;
//      int sizeOfIncidenceArray=0;
//      string sentenceid="";
//      string sourceIncidenceId="";
//      int globalSize=incidenceArray.size();
//      for(i=0;i<100;i++)
//      {
//              try{
//                      //source Incidence will be where the to be moved sentence belong to
//                      //sourceIncidence=generateRandomInteger(0,xlength-1);
//                          cout<<"index i is: "<<i<<endl;
//             sizeOfIncidenceArray=incidenceArray.size();
//             //cout<<"size of incidence array is: "<<sizeOfIncidenceArray<<endl;
//             sourceIncidenceIndex=generateRandomInteger(0,sizeOfIncidenceArray-1);
//             Incidence sourceIncidence=*(incidenceArray[sourceIncidenceIndex]);
//             sourceIncidenceId=sourceIncidence.inci_id;
//                          int size=sourceIncidence.sentencesid.size();
//             if(size==0)
// {
//               continue;
//             }
//                      //cout<<"size: "<<size<<endl;
//                      //cout<<"size: "<<size<<endl;
//                      // if(size==0)
//                      // {
//                      //      continue;
//                      // }
//                      sentenceToMove=generateRandomInteger(0,size-1);

//                      sentenceid=sourceIncidence.sentenc:/esid[sentenceToMove];
//                      cout<<"sentenceid: "<<sentenceid<<endl;
//              incidenceDestinationIndex=generateRandomInteger(0,sizeOfIncidenceArray-1);
//              incidenceDestination=(*(incidenceArray[incidenceDestinationIndex])).inci_id;

//              linkSentenceToIncidence(incidenceDestinationIndex,incidenceDestination,sourceIncidenceIndex,sourceIncidenceId,sentenceid,sentenceToMove,matrix,0.5,incidenceArray,subincidenceArray);
//              }
//              catch (...)
//              {
//                  // catch anything thrown within try block that derives from std::exception
//                  cout<<"what is the error???"<<i<<endl;
//                  //cout << exc.what();
//              }
//      }
//      //let see how many incidence left in incidenceArray
//      cout<<"incidence get left is here: "<<incidenceArray.size()<<endl;
//      //to see what incidence has the most sentenceId. to see what is the max sentence count in the incidence.
//      unsigned long maxSentenceCount=0;
//      Incidence* needToCheck;
//      for(Incidence* inc:incidenceArray)
//      {
//        if(maxSentenceCount<(*inc).sentencesid.size())
//        {
//         needToCheck=inc;
//        }

//                 maxSentenceCount=max(maxSentenceCount,(*inc).sentencesid.size());

//      }
//      // cout<<"incidenceid looking at is: "<<(*needToCheck).inci_id<<endl;
//      // cout<<"max sentenceids is this: "<<maxSentenceCount<<endl;
//      for(string index :(*needToCheck).sentencesid)
//      {
//       cout<<"sentenceid to check: "<<index<<endl;
//      }
//      // cout<<"similarirty: "<<getSimilarityByMatrixIndex(matrix,stoi((*needToCheck).sentencesid[0]),stoi((*needToCheck).sentencesid[1]))<<endl;
//      // cout<<"similarirty: "<<getSimilarityByMatrixIndex(matrix,stoi((*needToCheck).sentencesid[1]),stoi((*needToCheck).sentencesid[2]))<<endl;
//      // cout<<"similarirty: "<<getSimilarityByMatrixIndex(matrix,stoi((*needToCheck).sentencesid[0]),stoi((*needToCheck).sentencesid[2]))<<endl;

//      //test generateRandomInteger,
//      vector<int> test=*(splitTheIntegerIntoRandomPart(17));
//      for(int i: test)
//      {
//              cout<<"hey number"<<i<<endl;
//      }
//      cout<<"test size is: "<<test.size()<<endl;
// vector<int> ressult=shuffleTheIndexOfVector(10);
// if( __cplusplus == 201103L ) std::cout << "C++11\n" ;
// else if( __cplusplus == 19971L ) std::cout << "C++98\n" ;
// else std::cout << "pre-standard C++\n" ;
// //  return 0;

// //test guid string generator


// }
int main(int argc, char** argv)
{
    //initialize global feature weight
    GlobalFeatureWeight globalFeatureWeight;
    cout << globalFeatureWeight.featureWeight["code"] << endl;
    bool alive = true;
    vector<Sentence *> sentenceArray;
    //the score threshhold to make a decision link or not link
    int score=atoi(argv[1]);
    //iteration times
    int iteration=atoi(argv[2]);

    cout<<"score threshold is: "<<score<<endl;
    cout<<"No of iterations: "<<iteration<<endl;
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

                SentenceFeatureValue *value = new SentenceFeatureValue(code,  date8,  id, year, src_actor, src_agent, tgt_actor, tgt_agent, month, day,embed3, i);
                sentenceArray.push_back(new Sentence(id, value));


            }
        }
        alive = false;
    }

    int *vec1 = (*((*(sentenceArray[10000])).featureValue)).embed;
    int *vec2 = (*((*(sentenceArray[9999])).featureValue)).embed;
    double cosine = cosineSimilarity(vec1, vec2);
    cout << cosine << endl;

    // time_t now = time(0);

    // convert now to string form
    //    char *dt = ctime(&now);
    // srand(time(0));

    //  cout << "The local date and time is: " << dt << endl;
    //loadMatrix();
    /*load the simulated probability matrxi.*/
    //double *matrix = loadMatrix();


    //int i = 0;
    //Sentence* sentenceArray[xlength];,
    //vector<Sentence *> sentenceArray;
    vector<Incidence *> incidenceArray;
    vector<Subincidence *> subincidenceArray;
    //Incidence* incidenceArray[xlength];

    // for(int i = 0; i < xlength; i++)
    // {
    //     sentenceArray.push_back(new Sentence(to_string(i)));
    // }
    //initialize all the incidence.
    // for(int i = 0; i < xlength; i++)
    // {
    //     //this will allocate on the stack.
    //     vector<string> sentencesid;
    //     sentencesid.push_back(to_string(i));
    //     incidenceArray.push_back(new Incidence(to_string(i), sentencesid));
    // }

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
    //cout << "last active: " + to_string(lastActiveIncidenceIndex) << endl;
    int linkedcount = 0;
    for(int i = 0; i < iteration; i++)
    {
        try
        {
            //source Incidence will be where the to be moved sentence belong to
            //sourceIncidence=generateRandomInteger(0,xlength-1);
            //cout << "index i is: " << i << endl;
            sizeOfIncidenceArray = incidenceArray.size();
            //set the index of the last active incidence

            //cout<<"size of incidence array is: "<<sizeOfIncidenceArray<<endl;
            sourceIncidenceIndex = generateRandomInteger(0, sizeOfIncidenceArray - 1);
            Incidence sourceIncidence = *(incidenceArray[sourceIncidenceIndex]);
            sourceIncidenceId = sourceIncidence.inci_id;
            int size = sourceIncidence.sentencesid.size();
            //ToFo:if there is no sentence in the incidence we need to replace the tail incidence with the current one.
            if(size == 0)
            {
                //sawp the incidence with the last one
                incidenceArray[sourceIncidenceIndex] = incidenceArray[lastActiveIncidenceIndex];
                //swap the last active incidenceIndex at the empty spot, then decreas the lastActiveIncidenceIndex.
                lastActiveIncidenceIndex = lastActiveIncidenceIndex - 1;
                continue;
            }
            //otherwise choose a sentence to move
            int sentenceIndexInSource = generateRandomInteger(0, size - 1);
            //in the sentencesid store the index of the global sentence array.
            int sentenceGlobalIndex = sourceIncidence.sentencesid[sentenceIndexInSource];
            //cout << to_string(sentenceIndexInSource) + " " + to_string(sentenceGlobalIndex) << endl;
            destinationIncidenceIndex = generateRandomInteger(0, sizeOfIncidenceArray - 1);
            incidenceDestination = (*(incidenceArray[ destinationIncidenceIndex])).inci_id;

            double originalSimilarity = getSentenceSimilarityWithinIncidence(sentenceArray, incidenceArray, sourceIncidenceIndex, sentenceIndexInSource);
            double newSimilarity = getSentenceSimilarityWithinIncidence(sentenceArray, incidenceArray, destinationIncidenceIndex, sentenceIndexInSource);
            double originalPairs = getPropertyValueMatch(sentenceArray, incidenceArray, sourceIncidenceIndex, sentenceGlobalIndex, true, score);
            double newPairs = getPropertyValueMatch(sentenceArray, incidenceArray, destinationIncidenceIndex, sentenceGlobalIndex, false,score);
            //using the metroplis hastings algorithms here
            double originalFinalScore = (1.0 / 13.0) * originalSimilarity + (12.0 / 13.0) * originalPairs;
            double newFinalScore = (1.0 / 13.0) * newSimilarity + (12.0 / 13.0) * newPairs;
            //double mh_value = min(1.0, originalFinalScore / newFinalScore);
            double mh_value = min(1.0, originalSimilarity / newSimilarity);

            //not link if 3 pairs not match for the new configureations
            if(newPairs == 10)
            {
                cout << "mh value: " << mh_value << endl;
                linkedcount++;
                //cout << "dest: " << destinationIncidenceIndex << endl;
                //cout << "sen: " << sentenceGlobalIndex << endl;
                linkSentenceToIncidence(incidenceArray, destinationIncidenceIndex, sourceIncidenceIndex, sentenceGlobalIndex, sentenceIndexInSource);

                // if(mh_value == 1)
                // {
                //     linkedcount++;
                //     linkSentenceToIncidence(incidenceArray, destinationIncidenceIndex, sourceIncidenceIndex, sentenceGlobalIndex, sentenceIndexInSource);
                // }
                // else
                // {
                //     //mh_value is the probablity to link the two incidence together.
                //     int randomnum = generateRandomInteger(1, 100);
                //     if((randomnum / 100.0) < mh_value)
                //     {
                //         //then make the move
                //         linkedcount++;
                //         linkSentenceToIncidence(incidenceArray, destinationIncidenceIndex, sourceIncidenceIndex, sentenceGlobalIndex, sentenceIndexInSource);
                //     }

                // }

            }
            //if the new similarity is bigger, then make the link

        }
        catch (...)
        {
            // catch anything thrown within try block that derives from std::exception
            cout << "what is the error???" << i << endl;
            //cout << exc.what();
        }
    }
    cout << "linked count: " + to_string(linkedcount) << endl;
    cout << "last active later: " + to_string(lastActiveIncidenceIndex) << endl;
    int count = 0;
    for(int i = 0; i < lastActiveIncidenceIndex; i++)
    {
        vector<int> sentencesid = (*(incidenceArray[i])).sentencesid;
        if(sentencesid.size() > 1)
        {
            count++;
            for(unsigned int j = 0; j < sentencesid.size(); j++)
            {
                int curr = sentencesid[j];
                string realid = (*((*(sentenceArray[curr])).featureValue)).id;
                cout << realid << endl;

            }
            cout << " " << endl;
            cout << " " << endl;
            //return 0;
        }
        if(count >= 5)
        {
            return 0;
        }

    }

    //let see how many incidence left in incidenceArray
    // cout << "incidence get left is here: " << incidenceArray.size() << endl;
    // //to see what incidence has the most sentenceId. to see what is the max sentence count in the incidence.
    // unsigned long maxSentenceCount = 0;
    // Incidence *needToCheck;
    // for(Incidence *inc : incidenceArray)
    // {
    //     if(maxSentenceCount < (*inc).sentencesid.size())
    //     {
    //         needToCheck = inc;
    //     }

    //     maxSentenceCount = max(maxSentenceCount, (*inc).sentencesid.size());

    // }
    // // cout<<"incidenceid looking at is: "<<(*needToCheck).inci_id<<endl;
    // // cout<<"max sentenceids is this: "<<maxSentenceCount<<endl;
    // for(string index : (*needToCheck).sentencesid)
    // {
    //     cout << "sentenceid to check: " << index << endl;
    // }
    // // cout<<"similarirty: "<<getSimilarityByMatrixIndex(matrix,stoi((*needToCheck).sentencesid[0]),stoi((*needToCheck).sentencesid[1]))<<endl;
    // // cout<<"similarirty: "<<getSimilarityByMatrixIndex(matrix,stoi((*needToCheck).sentencesid[1]),stoi((*needToCheck).sentencesid[2]))<<endl;
    // // cout<<"similarirty: "<<getSimilarityByMatrixIndex(matrix,stoi((*needToCheck).sentencesid[0]),stoi((*needToCheck).sentencesid[2]))<<endl;

    // //test generateRandomInteger,
    // vector<int> test = *(splitTheIntegerIntoRandomPart(17));
    // for(int i : test)
    // {
    //     cout << "hey number" << i << endl;
    // }
    // cout << "test size is: " << test.size() << endl;
    // vector<int> ressult = shuffleTheIndexOfVector(10);
    // if( __cplusplus == 201103L ) std::cout << "C++11\n" ;
    // else if( __cplusplus == 19971L ) std::cout << "C++98\n" ;
    // else std::cout << "pre-standard C++\n" ;
    return 0;
}






