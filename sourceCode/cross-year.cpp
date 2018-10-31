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

void rtrim(std::string &);

class SentenceFeatureValue
{
public:
    //map<string,string> featureValue;
    string code;
    string rootcode;
    double latitude;
    double longitude;
    string geoname;
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
    SentenceFeatureValue()
    {

    }
    SentenceFeatureValue(string code1, string rootcode1, string date81,  string id1, string year1, string src_actor1, string src_agent1, string tgt_actor1, string tgt_agent1, string month1, string day1, int *embed1, int index1,double latitude,double longitude, string geoname)
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
        //rtrim(latitude);
        //rtrim(longitude);
        rtrim(geoname);
    }
};

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
         cout<<parsedindex<<word<<endl;
         (*v).index=stoi(word); 
        }
        break;
      case 16: embedstring=word;break;
      }
    }
    Sentence* sentence=new Sentence((*v).id,v,incidenceIndex);
    sentenceArray.push_back(sentence);
    (*v).index=sentenceIndex;
    (*v).embed=new int[EMBED_SIZE];

    int innerindex=0;
    stringstream ss(embedstring);
    while(getline(ss,inword,'|'))
    {
       (*v).embed[innerindex]=stoi(inword);
       innerindex++;
    }
    
    //each incidence has a sentences array.
    incidence.sentencesid.push_back(sentenceIndex);
    //sentenceArray.push_back(sentence);

}


int main(int argc, char *argv[])
{
  ifstream in("test1977.rst");

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
  while(in) {
    getline(in, data);  // delim defaults to '\n'
    //' ' is the character that I wrote to the file.
   
     if (data==" ")
        {
            //when there is a newline with " ", we need to create a new incidence
            //incidence=new Incidence();
            incidenceIndex++;
            vector<int>* sentencesid=new vector<int>();
            incidenceArray.push_back(new Incidence(incidenceIndex,(*sentencesid)));
            incidence_pointer=incidenceArray[(incidenceArray).size()-1];
            //incidence=*(incidence_pointer);
            cout << "Empty line." << endl;
            //sentencesid=new vector<int>();
            //(*incidence_pointer).sentencesid=(*sentencesid);
            cout<<"here shoukd generate a new incidence!"<<endl;
            continue;
        }
        else
        {
          deserializeIncidence(data,*incidence_pointer, ref(incidenceArray),ref(sentenceArray));
        }
  }

  in.close();
  cout<<(incidenceArray).size()<<endl;
  cout<<(sentenceArray).size()<<endl;
  return 0;
}
