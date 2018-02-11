#include <iostream>
#include <pthread.h>
#include <string>
#include <ctime>
#include <vector>
#include <fstream>
#include <stdlib.h>
#include <algorithm>
#include <array> 
#include <random>       // std::default_random_engine
#include <chrono>       // std::chrono::system_clock

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

        Sentence(string sentenceid):sen_id(sentenceid) {};
};

class Incidence
{
	public:
		string inci_id;
		string sup_id;
		/*will be a list of snetence id that is in the incidence*/
		vector<string> sentencesid;
		vector<string> subincidencesid;

	    Incidence(string incidenceid,vector<string> sentences):inci_id(incidenceid),sentencesid(move(sentences)){};
};

class Subincidence
{
	public: 
		string sub_id;
		string inci_id;
		/*****list of features that subincidence care about*/;
		Subincidence(string subid,string inciid):sub_id(subid),inci_id(inciid){}
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

//give an integer split it into several other integers randomly and will add up to the integer
vector<int>* splitTheIntegerIntoRandomPart(int sum)
{
	vector<int>* randomNumberArray=new vector<int>();
	int numberGenerate=0;
	while(sum>0)
	{
		numberGenerate=generateRandomInteger(0,sum);
		//continue if the numberGenerated is 0, since that is useless
		if(numberGenerate==0)
		{
			continue;
		}
		(*randomNumberArray).push_back(numberGenerate);
		sum=sum-numberGenerate;

	}
	return randomNumberArray;
}

//split the target incidence if new stuff get added in , 
//the current thought might need to change later.
void splitIncidenceIntoSubincidence(int incidenceIndex, string incidenceId, vector<Subincidence*>& subincidenceArray,vector<Incidence*>& incidenceArray)
{
	
	//randomly split the sn
	int sentencesCount=(*incidenceArray[incidenceIndex]).sentencesid.size();
	vector<int> randomArray=*splitTheIntegerIntoRandomPart(sentencesCount);
	//might give it an probablity to split or not.
	//first shuffle the list then, make them in to subgroup, shuffle is provided by c++ native lib.
	for(int num:randomArray)
	{
		//cout<<"num to be split is:"<<num<<endl;


	}


}

vector<int>& shuffleTheIndexOfVector(int n)
{
	//shuffle the number ={0,1,2,3.....n-1}
	 //std::array<int,5> foo={1,2,3,4,5};
	vector<int> * random=new vector<int>();
	for(int i=0;i<n;i++)
	{
		(*random).push_back(i);
	}
    // obtain a time-based seed:
	srand(std::chrono::system_clock::now().time_since_epoch().count());
	random_shuffle((*random).begin(), (*random).end());
	cout << "startShuffle:"<<endl;
	  for (int& x: (*random)) 
	  	cout << x<<endl;

	return (*random);
}

/**link a sentence to a coincidence*, when bigger than the threshold the stuff should be moved*/
/***array is by default pass by reference**/
void linkSentenceToIncidence(int desincidenceindex,string incidenceid, int sourceincidenceindex,string sourceincidenceid,string sentenceid,int indexOfSentenceId,double* matrix,double threshold,vector<Incidence*>& incidenceArray,vector<Subincidence*>& subincidenceArray)
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
    	//remove from the old incidence and add into the new incidence.
        vector<string>& sentenceids=(*(incidenceArray[sourceincidenceindex])).sentencesid;
        sentenceids.erase(sentenceids.begin()+indexOfSentenceId);
        //if there is no sentence inside of the incidence any more, remove the incidence from the incidence list
        if(sentenceids.size()==0)
        	incidenceArray.erase(incidenceArray.begin()+sourceincidenceindex);

         //add the sentence to the destination incidence

        (*(incidenceArray[desincidenceindex])).sentencesid.push_back(sentenceid);
        splitIncidenceIntoSubincidence(desincidenceindex,incidenceid,subincidenceArray,incidenceArray);

    }
    else
    {
    	cout<<"not linked!!"<<endl;
    }
    //now need to make the linked sentecnes into the incidence list, and need to get rid of the incidence if there is nothing belong to it any more,


    
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
     //Sentence* sentenceArray[xlength];, 
     vector<Sentence*> sentenceArray;
     vector<Incidence*> incidenceArray;
     vector<Subincidence*> subincidenceArray;
     //Incidence* incidenceArray[xlength]; 

     for(int i=0;i<xlength;i++)
     {
        sentenceArray.push_back(new Sentence(to_string(i)));
     }
     //initialize all the incidence.
     for(int i=0;i<xlength;i++)
     {
     	vector<string> sentencesid;
     	sentencesid.push_back(to_string(i));
     	incidenceArray.push_back(new Incidence(to_string(i),sentencesid));
     }
     
   //  cout<<"sentenceid in the incidence: "<<(*(incidenceArray[11])).sentencesid[0]<<endl;
     int sentenceToMove=0;
     int incidenceDestinationIndex=0;
     //this will be the incidenceid.
     string incidenceDestination="";
     int sourceIncidenceIndex=0;
     int sizeOfIncidenceArray=0;
     string sentenceid="";
     string sourceIncidenceId="";
     for(i=0;i<200;i++)
     {
     	try{
     		//source Incidence will be where the to be moved sentence belong to
     		//sourceIncidence=generateRandomInteger(0,xlength-1);
            sizeOfIncidenceArray=incidenceArray.size();
            sourceIncidenceIndex=generateRandomInteger(0,sizeOfIncidenceArray-1);
            Incidence sourceIncidence=*(incidenceArray[sourceIncidenceIndex]);
            sourceIncidenceId=sourceIncidence.inci_id;
     		int size=sourceIncidence.sentencesid.size();
     		//cout<<"size: "<<size<<endl;
     		//cout<<"size: "<<size<<endl;
     		// if(size==0)
     		// {
     		// 	continue;
     		// }
     		sentenceToMove=generateRandomInteger(0,size-1);

     		sentenceid=sourceIncidence.sentencesid[sentenceToMove];
     		cout<<"sentenceid: "<<sentenceid<<endl;
	     	incidenceDestinationIndex=generateRandomInteger(0,xlength-1);
	     	incidenceDestination=(*(incidenceArray[incidenceDestinationIndex])).inci_id;
	     	linkSentenceToIncidence(incidenceDestinationIndex,incidenceDestination,sourceIncidenceIndex,sourceIncidenceId,sentenceid,sentenceToMove,matrix,0.5,incidenceArray,subincidenceArray);
     	}
     	catch (...)
		{
		    // catch anything thrown within try block that derives from std::exception
		    cout<<"what is the error???"<<i<<endl;
		    //cout << exc.what();
		}
     }
     //let see how many incidence left in incidenceArray
     cout<<"incidence get left is here: "<<incidenceArray.size()<<endl;
     //to see what incidence has the most sentenceId.
     unsigned long maxSentenceCount=0;
     for(Incidence* inc:incidenceArray)
     {
     	   maxSentenceCount=max(maxSentenceCount,(*inc).sentencesid.size());

     }
     cout<<"max sentenceids is this: "<<maxSentenceCount<<endl;

     //test generateRandomInteger, 
     vector<int> test=*(splitTheIntegerIntoRandomPart(17));
     for(int i: test)
     {
     	cout<<"hey number"<<i<<endl;
     }
     cout<<"test size is: "<<test.size()<<endl;
vector<int> ressult=shuffleTheIndexOfVector(10);
//check the version of c++ that using here.
if( __cplusplus == 201103L ) std::cout << "C++11\n" ;
else if( __cplusplus == 19971L ) std::cout << "C++98\n" ;
else std::cout << "pre-standard C++\n" ;
//  return 0;
	
}

