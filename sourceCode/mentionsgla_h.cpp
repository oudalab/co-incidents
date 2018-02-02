#ifndef _MENTIONS_GLA_H
#define _MENTIONS_GLA_H
// STL includes
#include <map>
#include <math.h>
#include <vector>
#include <algorithm>
#include "Random.h"
// DataPath library includes
#include "base/Types/INT.h"
#include "base/Types/STRING.h"
#define USE_TR1

using namespace std;

#ifdef USE_TR1
#include <tr1/unordered_map>
#include <tr1/unordered_set>
using namespace std::tr1;
#define DEFINED_MAP std::tr1::unordered_map
#define DEFINED_SET std::tr1::unordered_set
#endif // USE_TR1

#ifdef USE_MCT
#include <mct/hash-map.hpp>
#include <mct/hash-set.hpp>
using namespace mct;
#define DEFINED_MAP mct::closed_hash_map
#define DEFINED_SET mct::closed_hash_set
#endif // USE_MCT


/*  System Description of GLA
 *  GLA_DESC
 *      NAME(</MentionsGLA/>)
 * 	    INPUTS(</(mid, INT), (token, VARCHAR), (T0, INT), (T1, INT), (T2, INT), (T3, INT), (T4, INT), (T5, INT), (T6, INT), (T7, INT), (T8, INT), (T9, INT), (str_entid, INT)/>)
 *  	OUTPUTS(</(stringL, STRING ), (mid, INT)/>)
 *      RESULT_TYPE(</multi/>)
 *  END_DESC
 */
#define maxtoken 10
#define maxtokenlen 50

using namespace std;
typedef DEFINED_SET<size_t> u_set;
typedef DEFINED_SET<size_t>::iterator u_set_iter;

class Mention {
public:
    vector<int> tokenVec;
    char stringL[maxtokenlen+1];
    int mid; 
    int eid;
  
    Mention(const INT& men_id, const INT& ent_id, const VARCHAR& token, const vector<int>& tmpv) {
        memset(stringL,'\0',maxtokenlen+1);
        memcpy(stringL,token.ToString(),maxtokenlen+1);
        stringL[maxtokenlen]='\0';
        assert(strlen(token.ToString())>=1);           
        mid = men_id;
        eid = ent_id;
        for(int i=0; i<tmpv.size(); i++)
            tokenVec.push_back(tmpv[i]); 
    }

    double pairwiseScore(Mention& other) {
        double common=0; 
        int src=0;
        int des=0;

        while(true){
            if(tokenVec[src] > other.tokenVec[des]){ 
                des++;
            }
            else if(tokenVec[src] < other.tokenVec[des]){ 
                src++;
            }
            else {
                common++;
                des++;
                src++;
            }
            if(src == tokenVec.size() || des == other.tokenVec.size())
                break;
        }
        //cout<<"mention src id="<<mid<<" des id="<<other.mid<< " pairwsie="<<common/(sqrt(tokenVec.size())*sqrt(other.tokenVec.size()))<<endl;
        //cout<<"mention pairwsie="<<common<<endl;
        //cout<<"mention pairwsie="<<tokenVec.size()<<" "<<other.tokenVec.size()<<endl;
        return common/(sqrt(tokenVec.size())*sqrt(other.tokenVec.size()));
    }
};


class Sub_Entity {
    public:
        int sub_eid;
        vector<int> sub_mentions;
        DEFINED_SET<int> unique_tokens;        

        double pairwiseScore(Sub_Entity& other, vector<Mention>& mentions){
            double sum=0;
            int beg_index = RandInt()%(sub_mentions.size()-(int)sqrt(sub_mentions.size())+1);       
            int end_index = beg_index + (int)sqrt(sub_mentions.size());
            int other_beg_index = RandInt()%(other.sub_mentions.size()-(int)sqrt(other.sub_mentions.size())+1);       
            int other_end_index = other_beg_index + (int)sqrt(other.sub_mentions.size());

            for(int i=beg_index; i<end_index; i++)
                for(int j=other_beg_index; j<other_end_index; j++)
                    sum+=mentions[sub_mentions[i]].pairwiseScore(mentions[other.sub_mentions[j]]);
            return sum/((int)sqrt(sub_mentions.size())*(int)sqrt(other.sub_mentions.size()*1.0));
        }
};

class SuperEntity{
    public:
        int seid;
        int num_segs;
        DEFINED_SET<int> entities;
        vector<DEFINED_SET<int>> hash_entities;
        vector<pthread_mutex_t>  mutexs;

        SuperEntity() {
            seid=-1;
            num_segs=-1;
        }

        ~SuperEntity() {
            for(int index=0; index<mutexs.size(); index++)
                pthread_mutex_destroy(&mutexs[index]);
        }

        void lock(int index) {
            pthread_mutex_lock(&mutexs[index]);
        }

        void unlock(int index) {
            pthread_mutex_unlock(&mutexs[index]);
        }

        void finalize(){
            num_segs = entities.size()/10 + 1;
            mutexs.resize(num_segs);
            hash_entities.resize(num_segs);
            for(int i=0; i<num_segs; i++){
                pthread_mutex_init(&mutexs[i], NULL);
            }
            for(auto it=entities.begin(); it!=entities.end(); it++)
                hash_entities[*it%num_segs].insert(*it);    

        }
};

typedef DEFINED_MAP<int, SuperEntity> SuperEntityMap;
typedef DEFINED_MAP<int, SuperEntity>::iterator super_ent_map_iter;
typedef vector<SuperEntity> SuperEntityVec;

typedef DEFINED_MAP<int, Sub_Entity> SubEntityMap;
typedef DEFINED_MAP<int, Sub_Entity>::iterator sub_ent_iter;

class Entity {
    public:
        int eid; 
        vector<int> subEntities;
        pthread_mutex_t  mutex;
        vector<int> superEntities;
        vector<int> tmp;


        void remove(int sub_eid_index, int sub_eid, vector<Mention>& mentions, SubEntityMap& subEntityMap, SuperEntityVec& superEntityVec){
            assert(subEntities[sub_eid_index] == sub_eid);
            subEntities.erase(subEntities.begin()+sub_eid_index);

            tmp.clear();
            for(int i=0; i<subEntities.size(); i++){
                for(auto it=subEntityMap[subEntities[i]].unique_tokens.begin(); it!=subEntityMap[subEntities[i]].unique_tokens.end(); it++)
                    if(find(tmp.begin(), tmp.end(), *it) == tmp.end())
                    tmp.push_back(*it);
            }

            assert(superEntities.size() >= tmp.size());
            if(superEntities.size() != tmp.size()){
                for(auto it=superEntities.begin(); it!=superEntities.end(); it++){
                    if(find(tmp.begin(), tmp.end(), *it) == tmp.end()){
                        superEntityVec[*it].lock(eid%superEntityVec[*it].num_segs);
                        superEntityVec[*it].hash_entities[eid%superEntityVec[*it].num_segs].erase(eid);
                        superEntityVec[*it].unlock(eid%superEntityVec[*it].num_segs);
                    }
                }
                superEntities.clear();
                superEntities.insert(superEntities.end(), tmp.begin(), tmp.end());
            }
        }       

        void insert(int sub_eid_index, int sub_eid, vector<Mention>& mentions, SubEntityMap& subEntityMap, SuperEntityVec& superEntityVec) {
            subEntities.push_back(sub_eid); 
            for(auto it=subEntityMap[sub_eid].sub_mentions.begin(); it!=subEntityMap[sub_eid].sub_mentions.end(); it++){
                mentions[*it].eid=eid;
            }

            for(auto it=subEntityMap[sub_eid].unique_tokens.begin(); it!=subEntityMap[sub_eid].unique_tokens.end(); it++){
                if(find(superEntities.begin(), superEntities.end(), *it) == superEntities.end()){ // adding new sub_entity leads to increase the super_entity
                    superEntities.push_back(*it);
                    superEntityVec[*it].lock(eid%superEntityVec[*it].num_segs);
                    superEntityVec[*it].hash_entities[eid%superEntityVec[*it].num_segs].insert(eid);
                    superEntityVec[*it].unlock(eid%superEntityVec[*it].num_segs);
                }
            }
        }

        Entity() {
            eid=-1;
            pthread_mutex_init(&mutex, NULL);
        }

        ~Entity() {
            pthread_mutex_destroy(&mutex);
        }

        void lock() {
            pthread_mutex_lock(&mutex);
        }

        void unlock() {
            pthread_mutex_unlock(&mutex);
        }
};

typedef vector<Mention> MentionVector;
typedef vector<Mention>::iterator men_v_iter;

//typedef DEFINED_MAP<int, Sub_Entity> SubEntityMap;
//typedef DEFINED_MAP<int, Sub_Entity>::iterator sub_ent_iter;

typedef DEFINED_MAP<int, Entity> EntityMap;
typedef vector<Entity> EntityVec;
typedef DEFINED_MAP<int, Entity>::iterator ent_map_iter;

class MentionsGLA {
    MentionVector mentionVector;
    SubEntityMap subEntityMap;
    EntityMap entityMap;
    SuperEntityMap superEntityMap;
    men_v_iter men_iter;   
    DEFINED_SET<int> stopwords;

    public:
    MentionsGLA() {
        /*string stop_word[]= {"i", "me", "my", "myself", "we", "our", "ours", "ourselves", "you", "your", "yours", "yourself", 
            "yourselves", "he", "him", "his", "himself", "she", "her", "hers", "herself", "it", "its", "itself", 
            "they", "them", "their", "theirs", "themselves", "what", "which", "who", "whom", "this", "that", "these", 
            "those", "am", "is", "are", "was", "were", "be", "been", "being", "have", "has", "had", "having", "do", 
            "does", "did", "doing", "a", "an", "the", "and", "but", "if", "or", "because", "as", "until", "while", 
            "of", "at", "by", "for", "with", "about", "against", "between", "into", "through", "during", "before", 
            "after", "above", "below", "to", "from", "up", "down", "in", "out", "on", "off", "over", "under", "again", 
            "further", "then", "once", "here", "there", "when", "where", "why", "how", "all", "any", "both", "each", 
            "few", "more", "most", "other", "some", "such", "no", "nor", "not", "only", "own", "same", "so", "than", 
            "too", "very", "s", "t", "can", "will", "just", "don", "should", "now"};*/
        int stop_word[]= {14, 130, 191, 326, 533, 224, 230, 243, 7508, 533, 217, 188};
        int stop_size = sizeof(stop_word)/sizeof(stop_word[0]);
        for(int i=0; i< stop_size; i++)
            stopwords.insert(stop_word[i]);
        //assert(stopwords.size() == 127);

        cout<<"constructor MentionsGLA"<<endl;
    }

    ~MentionsGLA() { cout<<"deconstructor MentionsGLA"<<endl; }

    void AddItem(const INT& mid, const VARCHAR& token, const INT& T0, const INT& T1, const INT& T2, const INT& T3, const INT& T4, 
                 const INT& T5, const INT& T6, const INT& T7, const INT& T8, const INT& T9, const INT& str_entid) {
        int tmp[] = {T0,T1,T2,T3,T4,T5,T6,T7,T8,T9}; 
        vector<int> tmpv;             
 
        for(int i=0; i<10; i++){
           if(tmp[i] != -1 && stopwords.find(tmp[i])==stopwords.end()) tmpv.push_back(tmp[i]);
        } 

        sort(tmpv.begin(),tmpv.end());
        Mention mention(mid, str_entid, token, tmpv);

        // build super entity data structure
        for(int i=0; i<tmpv.size(); i++){
            if(superEntityMap.find(tmpv[i]) == superEntityMap.end()){
                SuperEntity superEntity;
                superEntity.seid=tmpv[i];
                superEntity.entities.insert(str_entid);
                superEntityMap[tmpv[i]]=superEntity; 
            }
            else{
                superEntityMap[tmpv[i]].entities.insert(str_entid);
            }
        }

        // build string match entity data structure
        if(entityMap.find(str_entid) == entityMap.end()){
            Entity entity;
            entity.eid=str_entid;
            Sub_Entity sub_entity;
            sub_entity.sub_eid=str_entid; 
            sub_entity.sub_mentions.push_back(mention.mid);
            std::copy(tmpv.begin(), tmpv.end(), std::inserter(sub_entity.unique_tokens, sub_entity.unique_tokens.end()));
            for(int i=0; i<tmpv.size(); i++){
                if(find(entity.superEntities.begin(), entity.superEntities.end(), tmpv[i])==entity.superEntities.end())
                    entity.superEntities.push_back(tmpv[i]);
            }
           
            entity.subEntities.push_back(str_entid);
            
            subEntityMap[str_entid] = sub_entity; 
            entityMap[str_entid] = entity;  
        } else {
            // one entity is initialized with only one sub-entity
            subEntityMap[str_entid].sub_mentions.push_back(mention.mid);
        }

        // build mention data structure
        mentionVector.push_back(mention);
    }

    void AddState( MentionsGLA& other ) {
        // glue mention data
        mentionVector.insert( mentionVector.end(), other.mentionVector.begin(), other.mentionVector.end());

        // glue sub entity data
        for(sub_ent_iter iter=other.subEntityMap.begin(); iter!=other.subEntityMap.end(); iter++){
            if(subEntityMap.find(iter->first)==subEntityMap.end()){
                subEntityMap[iter->first]=iter->second;
            } else {
                subEntityMap[iter->first].sub_mentions.insert(
                        subEntityMap[iter->first].sub_mentions.end(),
                        other.subEntityMap[iter->first].sub_mentions.begin(), 
                        other.subEntityMap[iter->first].sub_mentions.end()
                        );
            }
        } 
        
         
        // glue entity data
        for(ent_map_iter iter=other.entityMap.begin(); iter!=other.entityMap.end(); iter++){
            if(entityMap.find(iter->first)==entityMap.end()){
                entityMap[iter->first]=iter->second;
            } 
        } 

        // glue super entity data
        for(super_ent_map_iter iter=other.superEntityMap.begin(); iter!=other.superEntityMap.end(); iter++){
            if(superEntityMap.find(iter->first)==superEntityMap.end()){
                superEntityMap[iter->first]=iter->second;
            } else {
                superEntityMap[iter->first].entities.insert(other.superEntityMap[iter->first].entities.begin(), 
                                                            other.superEntityMap[iter->first].entities.end());
            }
        } 

    }

    void GetMentionsBySwapping(MentionVector& other){
       MentionVector tempVector = mentionVector;
       for(int i=0; i< tempVector.size(); i++)
           mentionVector[tempVector[i].mid] = tempVector[i];
        mentionVector.swap(other);
    }

    void GetSubEntitiesBySwapping(SubEntityMap& other){
        subEntityMap.swap(other);
    }

    void GetEntitiesBySwapping(EntityVec& other){
        for(int i=0; i<entityMap.size(); i++)
            other.push_back(entityMap[i]);
    }

    void GetSuperEntitiesBySwapping(SuperEntityVec& other){
        for(int i=0; i<superEntityMap.size(); i++){
           superEntityMap[i].finalize();
           other.push_back(superEntityMap[i]);
        }
    }

    void Finalize( void ) {
        men_iter = mentionVector.begin();
    }

    bool GetNextResult( STRING& stringL, INT& mid) {
        return false;/* don't need to return any results */
        if( men_iter != mentionVector.end() ) {
            stringL = men_iter->stringL;
            mid = men_iter->mid;
            ++men_iter;
            return true;
        } else {
            return false;
        }
    }
};
#endif // _MENTIONS_GLA_H