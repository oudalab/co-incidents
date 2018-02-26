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
#include <uuid/uuid.h>

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
	//it hard to generate guid in c++, so maybe Subincidence don't need a guid 
		string sub_id;
		string inci_id;
		vector<string> sentencesid;
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

// void generateGuidString(string guidStr)
// {
// 	_TUCHAR *guidStr = 0x00;

// 	GUID *pguid = 0x00;

// 	pguid = new GUID; 

// 	CoCreateGuid(pguid); 

// 	// Convert the GUID to a string
// 	UuidToString(pguid, &guidStr);

// 	delete pguid;

// }


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

//split the target incidence if new stuff get added in , 
//the current thought might need to change later.
void splitIncidenceIntoSubincidence(int incidenceIndex, string incidenceId, vector<Subincidence*>& subincidenceArray,vector<Incidence*>& incidenceArray)
{
	
	//randomly split the sn
	vector<string> sentences=(*incidenceArray[incidenceIndex]).sentencesid;
	int sentencesCount=sentences.size();
	vector<int> randomArray=*splitTheIntegerIntoRandomPart(sentencesCount);
	//might give it an probablity to split or not.
	//first shuffle the list then, make them in to subgroup, shuffle is provided by c++ native lib.
	vector<int> shuffledIndex=shuffleTheIndexOfVector(sentencesCount);
	int sizeid=0;
	int accumulateIndex=0;
	int sentenceIndex=0;
	string subid="";
	for(int num:randomArray)
	{
		sizeid=subincidenceArray.size();
        subid=to_string(sizeid);
		Subincidence* sub=new Subincidence(subid,incidenceId);
		subincidenceArray.push_back(sub);
		for(int i=0;i<num;i++)
		{
			
			sentenceIndex=shuffledIndex[accumulateIndex];
			(*sub).sentencesid.push_back(sentences[sentenceIndex]);
			//be careful this needs to be called at last, otherwise it will get a segmentation fault
			accumulateIndex=accumulateIndex+1;
		}
		(*incidenceArray[incidenceIndex]).subincidencesid.push_back(subid);

	}
	cout<<"subcount: "<<(*incidenceArray[incidenceIndex]).subincidencesid.size()<<endl;


}


/**link a sentence to a coincidence*, when bigger than the threshold the stuff should be moved*/
/***array is by default pass by reference**/
void linkSentenceToIncidence(int desincidenceindex,string incidenceid, int sourceincidenceindex,string sourceincidenceid,string sentenceid,int indexOfSentenceId,double* matrix,double threshold,vector<Incidence*>& incidenceArray,vector<Subincidence*>& subincidenceArray)
{
    //let say when the sentece similarity within the average similarity for the coincidence is above some value then move.
    double sentenceWithIncidenceSimilarity=0;
    double similarityInOldIncidence=0;
    vector<string> sentencesid=(*(incidenceArray[stoi(incidenceid)])).sentencesid;
    //int count=0;
    int sen1=0;
    int sen2=stoi(sentenceid);
    //in order for the proces to start do this:
    if(sentencesid.size()==1)
    {
      sentenceWithIncidenceSimilarity=generateRandomInteger(0,100)/100.0;
    }
    else{
      sentenceWithIncidenceSimilarity=0;
        for(string id:sentencesid)
      {
        sen1=stoi(id);
        //count=count+1;
        //pairwisely calculate the similarity of the current add in stuff with the snetence already in the list and compare the similarity with some threshhold.
        sentenceWithIncidenceSimilarity=sentenceWithIncidenceSimilarity+getSimilarityByMatrixIndex(matrix,sen1,sen2);
      }

    }
    
    //now need to calculat sen2 affinity within its old icnidence
    vector<string> sourceSentencesid=(*(incidenceArray[stoi(sourceincidenceid)])).sentencesid;
    if(sourceSentencesid.size()==0)
    {
      similarityInOldIncidence=generateRandomInteger(0,100)/100.0;
    }
    else
    {
      similarityInOldIncidence=0;
        for(string id:sourceSentencesid)
      {
        sen1=stod(id);
        similarityInOldIncidence=similarityInOldIncidence+getSimilarityByMatrixIndex(matrix,sen1,sen2);
      }

    }
    
    //if(sentenceWithIncidenceSimilarity/count>=threshold)
    if(sentenceWithIncidenceSimilarity>=similarityInOldIncidence)
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
      //this will allocate on the stack.
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
     int globalSize=incidenceArray.size();
     for(i=0;i<100000;i++)
     {
     	try{
     		//source Incidence will be where the to be moved sentence belong to
     		//sourceIncidence=generateRandomInteger(0,xlength-1);
     		    cout<<"index i is: "<<i<<endl;
            sizeOfIncidenceArray=incidenceArray.size();
            //cout<<"size of incidence array is: "<<sizeOfIncidenceArray<<endl;
            sourceIncidenceIndex=generateRandomInteger(0,sizeOfIncidenceArray-1);
            Incidence sourceIncidence=*(incidenceArray[sourceIncidenceIndex]);
            sourceIncidenceId=sourceIncidence.inci_id;
     		    int size=sourceIncidence.sentencesid.size();
            if(size==0)
            {
              continue;
            }
     		//cout<<"size: "<<size<<endl;
     		//cout<<"size: "<<size<<endl;
     		// if(size==0)
     		// {
     		// 	continue;
     		// }
     		sentenceToMove=generateRandomInteger(0,size-1);

     		sentenceid=sourceIncidence.sentencesid[sentenceToMove];
     		cout<<"sentenceid: "<<sentenceid<<endl;
	     	incidenceDestinationIndex=generateRandomInteger(0,sizeOfIncidenceArray-1);
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
     //to see what incidence has the most sentenceId. to see what is the max sentence count in the incidence.
     unsigned long maxSentenceCount=0;
     Incidence* needToCheck;
     for(Incidence* inc:incidenceArray)
     {
       if(maxSentenceCount<(*inc).sentencesid.size())
       {
        needToCheck=inc;
       }

     	   maxSentenceCount=max(maxSentenceCount,(*inc).sentencesid.size());
       
     }
     // cout<<"incidenceid looking at is: "<<(*needToCheck).inci_id<<endl;
     // cout<<"max sentenceids is this: "<<maxSentenceCount<<endl;
     for(string index :(*needToCheck).sentencesid)
     {
      cout<<"sentenceid to check: "<<index<<endl;
     }
     // cout<<"similarirty: "<<getSimilarityByMatrixIndex(matrix,stoi((*needToCheck).sentencesid[0]),stoi((*needToCheck).sentencesid[1]))<<endl;
     // cout<<"similarirty: "<<getSimilarityByMatrixIndex(matrix,stoi((*needToCheck).sentencesid[1]),stoi((*needToCheck).sentencesid[2]))<<endl;
     // cout<<"similarirty: "<<getSimilarityByMatrixIndex(matrix,stoi((*needToCheck).sentencesid[0]),stoi((*needToCheck).sentencesid[2]))<<endl;

     //test generateRandomInteger, 
     vector<int> test=*(splitTheIntegerIntoRandomPart(17));
     for(int i: test)
     {
     	cout<<"hey number"<<i<<endl;
     }
     cout<<"test size is: "<<test.size()<<endl;
vector<int> ressult=shuffleTheIndexOfVector(10);
if( __cplusplus == 201103L ) std::cout << "C++11\n" ;
else if( __cplusplus == 19971L ) std::cout << "C++98\n" ;
else std::cout << "pre-standard C++\n" ;
//  return 0;

//test guid string generator

	
}

