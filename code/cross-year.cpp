#include <iostream>
#include <pthread.h>
#include <string>
#include <ctime>
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
#include <vector>
#include <pthread.h>
#include "GlobalFeatureWeight.h"
#include "Incidence.h"
#include "SubIncidence.h" //not getting used yet in this code.
#include "SuperIncidence.h" //not getting used yet in this code
#include "SharedResources.h"
#include "Sentence.h"
#include "SentenceFeatureValue.h"
#include "event.pb.h"
#include "Util.hpp"
using namespace std;


// #define EMBED_SIZE 150
// #define BOUND 100
int lastActiveIncidenceIndex = 0;


// void deserializeIncidence(string str,Incidence& incidence,vector<Incidence*> &incidenceArray, vector<Sentence*> &sentenceArray)
// {

//     stringstream stream(str);
//     int parsedindex=0;
//     int sentenceIndex=sentenceArray.size();
//     vector<int> sentencesid=incidence.sentencesid;
//     int incidenceIndex=incidenceArray.size();
//     SentenceFeatureValue* v=new SentenceFeatureValue();
    
//     string word;
//     string inword;
//     string embedstring;
//     while( getline(stream, word, ',') )
//     {
//       parsedindex++;
//       switch(parsedindex) {
//       case 1 : (*v).code=word;break;
//       case 2 : (*v).rootcode=word;break;
//       case 3 : 
//         if(word!=" "&&word!="")
//         {
//           try
//           {
//              (*v).latitude=stod(word);
//           }
//           catch(exception& e)
//           {
//             cout<<"exception encountered!"<<endl;
//           }
         
//         }
//         break;
//       case 4 : 
//         if(word!=" "&&word!="")
//         {
//           try
//           {
//             (*v).longitude=stod(word);
//           }
//           catch(exception& e)
//           {
//             cout<<"exception encountered!"<<endl;
//           }
//         }
//         break;
//       case 5 : (*v).geoname=word;break;
//       case 6 : (*v).date8=word;break;
//       case 7 : (*v).id=word;break;
//       case 8 : (*v).year=word;break;
//       case 9 : (*v).src_actor=word;break;
//       case 10: (*v).src_agent=word;break;
//       case 11: (*v).tgt_actor=word;break;
//       case 12: (*v).tgt_agent=word;break;
//       case 13: (*v).month=word;break;
//       case 14: (*v).day=word;break;
//       case 15: 
//         if(word!=" "&&word!="")
//         {
//          //cout<<parsedindex<<word<<endl;
//          (*v).index=stoi(word); 
//         }
//         break;
//       case 16: 
//                cout<<"you ever get here??"<<endl;
//                embedstring=word;
//                cout<<word<<endl;
//                break;
//       }
//     }
//    c
//     sentenceArray.push_back(sentence);
//     (*v).index=sentenceIndex;
//     (*v).embed=new int[EMBED_SIZE];

//     int innerindex=0;
//     cout<<embedstring<<endl;
//     stringstream ss(embedstring);
//     while(getline(ss,inword,'|'))
//     {
//        cout<<inword<<"||";
//        (*v).embed[innerindex]=stoi(inword);
//        innerindex++;
//     }
    
//     //each incidence has a sentences array.
//     incidence.sentencesid.push_back(sentenceIndex);
//     //sentenceArray.push_back(sentence);
// }
/*
*Transform the event from incidence into the sentence form that we will use
*return a pointer to this sentence.
*/
void Sentence* TransformEvent(const models::Event& event,int incidenceIndex)
{
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

    if(event.has_code())
    {
        code=event.code();
    }
    if(event.has_rootcode())
    {
        rootcode=event.rootcode();
    }
    if(event.has_latitude())
    {
        latitude=event.latitude();
    }
    if(event.has_longitude())
    {
        longitude=event.longitude();
    }
    if(event.has_geoname())
    {
        geoname=event.geoname();
    }
    if(event.has_date8())
    {
        date8=event.date8();
    }
    if(event.has_src_actor())
    {
        src_actor=event.src_actor();
    }
    if(event.has_src_agent())
    {
        src_agent=event.src_agent();
    }
    if(event.has_tgt_actor())
    {
        tgt_agent=event.tgt_actor();
    }
    if(event.has_tgt_agent())
    {
        tgt_agent=event.tgt_agent();
    }
    if(event.has_month())
    {
        month=event.month();
    }
    if(event.has_day())
    {
        day=event.day();
    }
    if(event.has_id())
    {
        id=event.id();
    }
    int embed = new int[EMBED_SIZE];
    if(event.has_embed())
    {
       for(int j=0;j<event.embed().str_size();j++)
       {
           embed[j]=stoi(event.embed().str(j));
       }
    }
    SentenceFeatureValue* v=new SentenceFeatureValue(code, root_code, date8, id, year, src_actor, src_other_agent,tgt_actor,tgt_agent, month, day, embed, row,latitude,longitude,geoname);

    Sentence* sentence=new Sentence(v->id,v,incidenceIndex);
}
/*
*it will decode the models::incidence into Incidence format and then create sentencesId for the Incidence
and add sentences and incidence to the sentenceArray and incidenceArray.
*/
 Incidence* TranformIncidence(const models::Incidence& incidence,vector<Incidence*> &incidenceArray, vector<Sentence*> &sentenceArray) {
  //cout<< incidence.event_size()<<endl;
  
  int incidenceIndex=incidenceArray.size();
  
  vector<int>* sentencesid=new vector<int>();

  Incidence * transformedIncidence=new Incidence(incidenceIndex, sentencesid);

  for (int i = 0; i < incidence.event_size(); i++) {
    const models::Event& event = incidence.event(i);

    int sentenceIndex=sentenceArray.size();

    sentencesid.push_back(sentenceIndex);

    Sentence* sentence=TransformEvent(ref(event),incidenceIndex);

    sentenceArray.push_back(sentence);
 }  
  return transformedIncidence;
}

/*
*Load in one incidence from disk and call decodeIncidence to transform to the Incidence type we use.
*/
void LoadIncidence(string incidence_file_name,vector<Incidence*> &incidenceArray, vector<Sentence*> &sentenceArray)
{ 
    models::Incidence incidence;
    cout<<incidence_file_name<<endl;
    // Read the existing address book.
    fstream input(incidence_file_name, ios::in | ios::binary);

    if(!incidence.ParseFromIstream(&input)) {   
       cerr<<"can not parse the incidence"<<endl;
       return;
    }
    DecodeIncidence(ref(incidence),ref(incidenceArray),ref(sentenceArray));
}

int main(int argc, char *argv[])
{
  string firstyear=std::string(argv[3]);
  string secondyear=std::string(argv[4]);
  ifstream in(firstyear+".rst");
  ifstream in2(secondyear+".rst");
  
  //the Incidence type should be the old one, not the serialzied one, since it has critical section in it.
  vector<Incidence*> incidenceArray;//=new vector<Incidence*>();
  vector<Sentence*> sentenceArray;//=new vector<Sentence*>();
  //char str[255];
  string data="";
  int incidenceIndex=0;
  
  //Incidence incidence;
  Incidence* incidence_pointer;
  // int linenumber=0;
  // while(in) {
    
  //   if(linenumber<=7)
  //   {
  //     getline(in, data);
  //     linenumber++;
  //     continue;
  //   }

  //   getline(in, data);
  //    if (data==" ")
  //       {
  //           incidenceIndex++;
  //           vector<int>* sentencesid=new vector<int>();
  //           incidenceArray.push_back(new Incidence(incidenceIndex,(*sentencesid)));
  //           incidence_pointer=incidenceArray[(incidenceArray).size()-1];
  //           continue;
  //       }
  //       else
  //       {
  //         deserializeIncidence(data,*incidence_pointer, ref(incidenceArray),ref(sentenceArray));
  //       }
  // }
  // in.close();
  //  //duplciate code can be extracted
  // linenumber=0;
  // while(in2) {
    
  //   if(linenumber<=7)
  //   {
  //     getline(in2, data);
  //     linenumber++;
  //     continue;
  //   }
  //     // delim defaults to '\n'
  //   //' ' is the character that I wrote to the file.
  //   getline(in2, data);
  //    if (data==" ")
  //       {
  //           incidenceIndex++;
  //           vector<int>* sentencesid=new vector<int>();
  //           incidenceArray.push_back(new Incidence(incidenceIndex,(*sentencesid)));
  //           incidence_pointer=incidenceArray[(incidenceArray).size()-1];
  //           continue;
  //       }
  //       else
  //       {
  //         deserializeIncidence(data,*incidence_pointer, ref(incidenceArray),ref(sentenceArray));
  //       }
  // }
  // in2.close();
  // int oldIncidenceArraySize=incidenceArray.size();
  // cout<<(incidenceArray).size()<<endl;
  for(int i=1;i<10;i++)
  {
    string dir="./data1980/test1980_"+std::to_string(i);
    LoadIncidence(dir, ref(incidenceArray),ref(sentenceArray));
  }
  //Optional:  Delete all global objects allocated by libprotobuf.
  google::protobuf::ShutdownProtobufLibrary();
  cout<<endl;
  cout<<"size of the incidence array: "<<incidenceArray.size()<<endl;
/****************************/
/***end of loading all the year data now need to start linking
*****************************/
    // string incidenceDestination = "";
    // string sentenceid = "";
    // string sourceIncidenceId = "";
    // int globalSize = incidenceArray.size();
    // lastActiveIncidenceIndex = globalSize - 1;
    // SharedResources *shared = new SharedResources(globalSize - 1);
    // clock_t begin = clock();
    // int score = atoi(argv[1]);
    // //well this not get used in link code or here just as a place holder here in that way I don't need to update the code
    // int iteration = atoi(argv[2]);
    // string statsfile = firstyear+"-"+secondyear + ".stas";
    // string outputfile = firstyear+"-"+secondyear + ".rst";
    
    // //using the dumb way this is not working.
    // // std::vector<std::thread> threads;
    // // for (int i = 0; i < 64; ++i) {
    // //     threads.push_back(thread(do_work_biased, ref(incidenceArray), ref(sentenceArray), ref(*shared), iteration, score, i+1, statsfile));
        
    // // }

    // // for (int i = 0; i < 64; ++i) {
    // //     threads[i].join();
    // // }
    
    // clock_t end = clock();
    // double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

    // //do_work_biased(incidenceArray,sentenceArray,*shared,iteration,score);
    // // cout << "linked count: " + to_string(linkedcount) << endl;
  
    // //int count = 0;
    // ofstream out(outputfile);
    // // out.open(outputfile);
    // if(!out)
    // {
    //     cout << "could not open file" << endl;
    //     return 0;
    // }

    // out << "time taken in seconds with " << iteration << " score" << score << " in seconds: " << elapsed_secs << endl;

    // cout << "last active when start: " << oldIncidenceArraySize-1<< endl;
    // cout << "last active when end: " + to_string((*shared).lastActiveIncidenceIndex) << endl;
    
    // out << "last active when start: " <<oldIncidenceArraySize-1<< endl;
    // out << "last active when end: " <<to_string((*shared).lastActiveIncidenceIndex) << endl;
    // cout<<"total linked:"<<shared->totallinked<<endl;
    // out<<"total linked:"<<shared->totallinked<<endl;
    // out<<" "<<endl;
    // for(int i = 0; i < (*shared).lastActiveIncidenceIndex; i++)
    // {
    //     vector<int> sentencesid = (*(incidenceArray[i])).sentencesid;
    //     if(sentencesid.size() > 1)
    //     {
    //         int curr1 = -1;
    //         int curr2 = -1;
    //         int curr3 = -1;
    //         int prev=sentencesid[0];

    //         for(unsigned int j = 0; j < sentencesid.size(); j++)
    //         {
    //             int curr = sentencesid[j];
    //             SentenceFeatureValue v=(*((*(sentenceArray[curr])).featureValue));

    //             out<<v.code<<","<<v.rootcode<<","<<v.latitude<<","<<v.longitude<<","<<v.geoname<<","
    //             <<v.date8<<","<<v.id<<","<<v.year<<","<<
    //             v.src_actor<<","<<v.src_agent<<","<<v.tgt_actor<<","<<v.tgt_agent<<","<<v.month<<","<<v.day<<","<<v.index<<",";
    //             for(int k=0;k<EMBED_SIZE;k++)
    //             {
    //                 out<<(v.embed)[k];
    //                 if(k!=EMBED_SIZE-1)
    //                 {
    //                     out<<"|";
    //                 }
    //             }
    //             out<<","<<sentencesid.size()<<endl;
    //         }
    //         out << " " << endl;
    //     }
    // }
  return 0;
}
