#include <iostream>
#include <pthread.h>
#include <string>
#include <ctime>
#include <vector>
#include <fstream>
#include <stdlib.h>

using namespace std;


class Sentence
{
	public:
		string sen_id;
		string location;
		string actor;
		string eventcode;
		 // current date/time based on current system
		string time_occur;
        vector<int> doc_embed;

        Sentence(string sentenceid):sen_id(sentenceid) {}
};

class Incidence
{
	public:
		string inci_id;
		string sup_id;
		/*will be a list of snetence id that is in the incidence*/
		vector<string> sentencesid;
		vector<string> subincidencesid;
};

class Subincidence
{
	public: 
		string sub_id;
		string inci_id;
		/*****list of features that subincidence care about*/
};

class SuperIncidence
{
	public:
		string sup_id;
		/* nodeid when distributed which node this superincidence is on*/
		string nodeid;
	  /****list of freatures that super incidence care about***/
};

int xlength=1000;
int length=(xlength*xlength-xlength)/2+xlength;

double* loadMatrix()
{
	int count = 0;
   
    double similarity [length];
    string line;
    
    cout << "Testing loading of file." << endl;
    ifstream myfile ("randommatrix");
    if ( myfile.is_open() )
    {
         while ( ! myfile.eof() )
         {
               getline (myfile, line);
               //cout<<"line: "+line<<endl;
               //logs.at(count) = line;
               try{
               	similarity[count]=stod(line);
               }
               catch(...)
               {
                //cout<<"the count is :"<<count<<endl<<endl<<endl;
               	return similarity;
               } 
               //cout<<"array: "<<similarity[count]<<endl;
               count++;
         }
         myfile.close();
    }else{
          cout << "Unable to open file." << endl;
    }
    /*still return the object on the stack*/
    return similarity;
}


int main()
{

	 time_t now = time(0);
	   
	   // convert now to string form
	 char* dt = ctime(&now);
	// srand(time(0));

	 cout << "The local date and time is: " << dt << endl;
	 //loadMatrix();

	 double* matrix=loadMatrix();

     
     int i=0;
     Sentence* sentenceArray[xlength];

     for(int i=0;i<xlength;i++)
     {
        sentenceArray[i]=new Sentence(to_string(i));
     }

     cout<<"print the id:"<<(*(sentenceArray[10])).sen_id<<endl;

	 // int index=0;
	 // for(index=0;index<length;index++)
	 // {
	 // 	cout<<"output: "<<matrix[index]<<endl;
	 // }
	 /***create length many sentences with id range from 0 to 500549*/

	
}

