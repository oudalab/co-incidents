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

	    Incidence(string incidenceid,vector<string> sentences):inci_id(incidenceid),sentencesid(move(sentences)){}
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

//global variables
int xlength=1000;
int length=(xlength*xlength-xlength)/2+xlength;
/**** ok, the goal of the simulatio for now is to cluster all the sentences that are similar to each otehr say who has the similarity bigger than .5*****/

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

double getSimilarityByMatrixIndex(double* matrix,int row, int col)
{ 
   if(row==col)
   	 return 1.0;
   	else if(row>col)
   	{
   		//return matrix[row*xlength+col];
   		// row is 0 then add 1
   		// row is 1 add in 1 and 2
   		// row is 2 then add in 1,2 and 3
   		int index=(1+row)*row/2+col;
   		return matrix[index];

   	}
   	else
   	{
   		//since it is symmetric 
   		return getSimilarityByMatrixIndex(matrix,col,row);
   	}
  
}

/*****generate a random number within a range,  include min and max value *****/
int generateRandomInteger(int min, int max)
{
	return min + (rand() % static_cast<int>(max - min + 1));
}


/**link a sentence to a coincidence*, when bigger than the threshold the stuff should be moved*/
/***array is by default pass by reference**/
void linkSentenceToIncidence(string incidenceid, string sentenceid,double* matrix,double threshold,Incidence* incidenceArray[xlength])
{
    //let say when the sentece similarity within the average similarity for the coincidence is above some value then move.
    double sentenceWithIncidenceSimilarity=0;
    vector<string> sentencesid=(*(incidenceArray[stoi(incidenceid)])).sentencesid;
    int count=0;
    int sen1=0;
    int sen2=stoi(sentenceid);
    for(string id:sentencesid)
    {
    	sen1=stoi(id);
    	count=count+1;
    	sentenceWithIncidenceSimilarity=sentenceWithIncidenceSimilarity+getSimilarityByMatrixIndex(matrix,sen1,sen2);
    }
    if(sentenceWithIncidenceSimilarity/count>=threshold)
    {
    	//if bigger than threshhold, then link it
    	cout<<"linked!!"<<endl;
    }
    else
    {
    	cout<<"not linked!!"<<endl;
    }
    
}
int main()

{

	 time_t now = time(0);
	   
	   // convert now to string form
	 char* dt = ctime(&now);
	// srand(time(0));

	 cout << "The local date and time is: " << dt << endl;
	 //loadMatrix();
     /*load the simulated probability matrxi.*/
	double* matrix=loadMatrix();

     
     int i=0;
     Sentence* sentenceArray[xlength];
     Incidence* incidenceArray[xlength]; 

     for(int i=0;i<xlength;i++)
     {
        sentenceArray[i]=new Sentence(to_string(i));
     }

     for(int i=0;i<xlength;i++)
     {
     	vector<string> sentencesid;
     	sentencesid.push_back(to_string(i));
     	incidenceArray[i]=new Incidence(to_string(i),sentencesid);
     }
     
   //  cout<<"sentenceid in the incidence: "<<(*(incidenceArray[11])).sentencesid[0]<<endl;
     int sentenceToMove=0;
     int incidenceDestination=0;
     for(i=0;i<200;i++)
     {
     	try{
     		sentenceToMove=generateRandomInteger(0,999);
	     	incidenceDestination=generateRandomInteger(0,999);
	     	linkSentenceToIncidence(to_string(sentenceToMove),to_string(incidenceDestination),matrix,0.5,incidenceArray);
     	}
     	catch (...)
		{
		    // catch anything thrown within try block that derives from std::exception
		    cout<<"what is the error???"<<endl;
		    //cout << exc.what();
		}
     }
	
}

