#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <iostream>
#include <limits>
#include <string>
#include <errno.h>
#include <time.h>
#include <cstring>

#define MAX(a,b) ( ((a) > (b)) ? (a) : (b) )
#define MIN(a,b) ( ((a) < (b)) ? (a) : (b) )

#define DOT std::cerr << ".";

// This only works for C
static const char *  currentTime () {
    time_t rawtime;
    struct tm * timeinfo;

    time (&rawtime);
    timeinfo = localtime (&rawtime);
    char * thetime = asctime(timeinfo);
    thetime[strlen(thetime)-1] = '\0';
    return (const char *) thetime;
    //return asctime(timeinfo);
}

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define DATE_STRING currentTime()

#define log_info(M) std::cerr << DATE_STRING << " [INFO] (" << __FILE__ << ":" << __LINE__ << ") | " << M  << "\n"

#define log_err(M) std::cerr << DATE_STRING << " [ERROR] (" << __FILE__ << ":" << __LINE__ << ": " << std::strerror(errno) << ") | " << M  << "\n"

#define check_mem(A) check((A), "Out of memory.")

void rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(),
                [](int ch) { 
                    return !std::isspace(ch);
                }).base(), s.end());
}

int dotProduct(int *vect_A, int *vect_B, int EMBED_SIZE)
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
    if ((vectorLength(vec1) * vectorLength(vec2)) == 0)
    {
        return 0;
    }
    return (dotProduct(vec1, vec2,EMBED_SIZE) * 1.0) / (vectorLength(vec1) * vectorLength(vec2));
}

/*
generate a random number within a range, include min and max value 
*/
int generateRandomInteger(int min, int max)
{
    srand(std::chrono::system_clock::now().time_since_epoch().count());
    return min + (rand() % static_cast<int>(max - min + 1));
}

/*
give an integer split it into several other integers randomly and will add up to the integer
*/
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

/*
shuffle the number ={0,1,2,3.....n-1}
*/
vector<int> &shuffleTheIndexOfVector(int n)
{
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

double getSimilarityBySentenceId( vector<Sentence *> &sentenceArray, int sen1index, int sen2index)
{
    int *vec1 = (*((*(sentenceArray[sen1index])).featureValue)).embed;
    int *vec2 = (*((*(sentenceArray[sen2index])).featureValue)).embed;
    double cosine = cosineSimilarity(vec1, vec2);
    return cosine;
}


double getSentenceSimilarityWithinIncidence(vector<Sentence *> &sentenceArray, vector<Incidence *> &incidenceArray, int incidenceid, int sentenceindex, bool fromsource)
{
    double sentenceWithIncidenceSimilarity = 0;
    vector<int> sentencesid = (*(incidenceArray[incidenceid])).sentencesid;
    int sen1 = 0;
    int sen2 = sentenceindex;
    //in order for the process to start do this:
    if(sentencesid.size() == 1 && fromsource)
    {
        sentenceWithIncidenceSimilarity = generateRandomInteger(0, 55) / 100.0;
        return sentenceWithIncidenceSimilarity;
    }
    else
    {
        for(int id : sentencesid)
        {
            if(id != sentenceindex)
            {
                sen1 = id;
                //pairwisely calculate the similarity of the current add in stuff with the snetence already in the list and compare the similarity with some threshhold.
                sentenceWithIncidenceSimilarity = sentenceWithIncidenceSimilarity + getSimilarityBySentenceId(sentenceArray, sen1, sen2);
            }
        }
    }
    if(!fromsource)
    {
        return sentenceWithIncidenceSimilarity / sentencesid.size();
    }
    else
    {
        return sentenceWithIncidenceSimilarity / (sentencesid.size() - 1);
    }
}

bool isTrival(string input)
{
    rtrim(input);
    if(input == "\"\"" || input == "" || input == " "||input=="00000" || input.empty() || input.length() == 0)
    {
        return true;
    }
    return false;
}


#endif  // UTIL_HPP
