#include <iostream>
#include <fstream>
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
#include <map>
#include <cstdlib>
#include <cmath> 

#include <mutex>
#include <thread>
#include <pthread.h>
using namespace std;

class Incidence
{
public:
    int inci_id; //should be the index in the incidence array so each time need to update it when something changed.
    pthread_mutex_t mutex;
    /*will be a list of snetence id that is in the incidence*/
    vector<string> sentencesid;
    vector<string> subincidencesid;
    //IncidenceFeature featureMap;
    Incidence(int incidenceid, vector<string> sentences)
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

int main(int argc, char *argv[])
{
  ifstream in("test1990.rst");

  if(!in) {
    cout << "Cannot open input file.\n";
    return 1;
  }
  vector<Incidence*>* incidenceArray=new vector<Incidence*>();
  //char str[255];
  string newid="";
  int incidenceIndex=0;
  vector<string>* sentencesid=new vector<string>();
  while(in) {
    getline(in, newid);  // delim defaults to '\n'
    //' ' is the character that I wrote to the file.
   
     if (newid==" ")
        {
            cout << "Empty line." << endl;
            Incidence* inc=new Incidence(incidenceIndex,*sentencesid);
            incidenceIndex++;
            (*incidenceArray).push_back(inc);
            sentencesid=new vector<string>();
            continue;
        }
        else
        {
           (*sentencesid).push_back(newid);
        }
    //if(in) cout << newid << endl;
  }

  in.close();
  cout<<incidenceIndex<<endl;
  return 0;
}
