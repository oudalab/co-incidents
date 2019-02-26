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

void ListEvents(const models::Incidence& incidence) {
  cout<< incidence.event_size()<<endl;
  for (int i = 0; i < incidence.event_size(); i++) {
    const models::Event& event = incidence.event(i);
    if(event.has_rootcode())
    {
      cout<<event.rootcode()<<endl;
    }
    if(event.has_src_actor())
    {
      cout<<event.src_actor()<<endl;
    }
    if(event.has_embed())
    {
       for(int j=0;j<event.embed().str_size();j++)
       {
           cout<<event.embed().str(j)<<",";
       }

    }
}	  
}
int main(int argc, char **argv)
{
	 models::Incidence incidence;

  
    // Read the existing address book.
    fstream input(argv[1], ios::in | ios::binary);
    if(!incidence.ParseFromIstream(&input)) {
       
       cerr<<"can not parse the incidence"<<endl;
    }
  ListEvents(incidence);
 
  // Optional:  Delete all global objects allocated by libprotobuf.
  google::protobuf::ShutdownProtobufLibrary();

  return 0;

}
