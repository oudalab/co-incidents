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
#include <chrono>    
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

const int xlength = 1000;
const int number_of_thread = 64;
int lastActiveIncidenceIndex = 0;
int length = (xlength * xlength - xlength) / 2 + xlength;

int main(int argc, char **argv)
{
    GlobalFeatureWeight globalFeatureWeight;
    vector<Sentence *> sentenceArray;

    // if you input two paramters the argc will be 3.
    if (argc < 6)
     {
         cout << "input the scorethreshold and also the sample number, filename, biased or not, and startdate, and enddate: " << endl;
         return 0;
     }

    int score = atoi(argv[1]);
    int iteration = atoi(argv[2]);
    string startyear = argv[3];
    string outputfile = std::string(argv[3]) + ".rst";
    string tstout = std::string(argv[3]) + ".tst";
    string statsfile = std::string(argv[3]) + ".stas";
    string startdate=argv[4];
    string enddate=argv[5];
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

    q[sizeof q - 1] = '\0';
    snprintf(
        q,
        sizeof q - 1,
        ("SELECT * FROM events WHERE date8 >='"+startdate+"' and date8<='"+enddate+"'").c_str()
    );

    if (sqlite3_open ("/home/lian9478/OU_Coincidence/coincidenceData/events1114.db", &db) != SQLITE_OK) {
        fprintf(stderr, "Error opening database.\n");
        return 2;
    }

    printf("Query: %s\n", q);

    sqlite3_prepare(db, q, sizeof q, &stmt, NULL);

    bool done = false;

    while (!done) {
        //printf("In select while\n");
        switch (sqlite3_step (stmt)) {
        case SQLITE_ROW:
        {
            try
            {
                bytes = sqlite3_column_bytes(stmt, 0);
                //this can be column 1,2 , 3 ....
                embed  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)); //embed
                latitude  = sqlite3_column_double(stmt, 1); //embed
                longitude = sqlite3_column_double(stmt, 2); //embed
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
                int *embed3 = new int[EMBED_SIZE];
                for(int j = 0; j < EMBED_SIZE; j++)
                {
                   embed3[j] = (int)embed.at(j*2+1);
                }
            
               SentenceFeatureValue *value = new SentenceFeatureValue(code, root_code, date8, id, year, src_actor, src_other_agent,tgt_actor,tgt_agent, month, day, embed3, row,latitude,longitude,geoname);
            
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
    vector<SubIncidence *> subincidenceArray;

    int sentencesSize = sentenceArray.size();

    //initialize each sentence as an incidence.
    for(int i = 0; i < sentencesSize; i++)
    {
        vector<int> *sentencesid = new vector<int>();
        (*sentencesid).push_back(i);
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

    clock_t end = clock();

    double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;

    ofstream out(outputfile);

    if(!out)
    {
        cout << "could not open file" << endl;
        return 0;
    }

    out << "time taken in seconds with " << iteration << " score" << score << " in seconds: " << elapsed_secs << endl;

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
        models::Incidence* incidence=new models::Incidence();
        for(unsigned int j = 0; j < sentencesid.size(); j++)
        {
            int curr = sentencesid[j];
            SentenceFeatureValue v=(*((*(sentenceArray[curr])).featureValue));
            models::Event* event = incidence->add_event();;
            string temp= std::string(startyear)+"_"+std::to_string(i)+"_"+v.src_actor+"_"+v.tgt_actor+"_"+v.src_agent+"_"+v.tgt_agent;
            fstream output(temp,ios::out | ios::trunc | ios::binary);
            event->set_code(v.code);
            event->set_rootcode(v.rootcode);
            event->set_latitude(v.latitude);
            event->set_longitude(v.longitude);
            event->set_geoname(v.geoname);
            event->set_date8(v.date8);
            event->set_id(v.id);
            event->set_year(v.year);
            event->set_src_actor(v.src_actor);
            event->set_src_agent(v.src_agent);
            event->set_tgt_actor(v.tgt_actor);
            event->set_tgt_agent(v.tgt_agent);
            event->set_month(v.month);
            event->set_day(v.day);
            event->set_index(v.index);
            
            std::vector<int> embedding;
            for(int k=0;k<EMBED_SIZE;k++)
             {
                embedding.push_back((v.embed)[k]);
             }

            models::Embed* embed = new models::Embed();
            //link https://stackoverflow.com/questions/684390/how-can-protocol-buffers-support-serialization-deserialization-of-std-containers
            vector<int>::iterator e = embedding.end();
            for (vector<int>::iterator i = embedding.begin(); 
                i != e; 
                ++i) {
              embed->add_str(*i); 
            }
            event->set_allocated_embed(embed);
            if (!incidence->SerializeToOstream(&output)) {
             out << "Failed to write address book." << endl;
              return -1;
            }    
        }
    }
    return 0;
}



