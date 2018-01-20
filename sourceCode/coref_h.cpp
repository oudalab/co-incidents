#ifndef COREFERENCE_H_
#define COREFERENCE_H_

/* Meta-information
 *
 *  GIST_DESC
 *      NAME(Coreference)
 *      OUTPUTS(</(men_id, INT), (ent_id, INT)/>)   // Fill this in!
 *      RESULT_TYPE(multi) // Fill this in!
 *
 *      TASK_TYPE(Task)
 *      LOCAL_SCHEDULER_TYPE(LocalScheduler)
 *      GLA_TYPE(CGLA)
 *
 *	REQ_CONST_STATES(</(mGLA, MentionsGLA)/>)
 *  END_DESC
 */
#include "coreference/GLAs/MentionsGLA.h"
#include <pthread.h>
#include <iostream>
#include <assert.h>

#include <sstream>
#define COREF_USE_MULTI 
//#define num_iter 2000
#define num_iter 20
//#define DEBUG


void printMention(const Mention& mm ) {
    cout << "<" << mm.stringL << "| " << mm.mid << " " << mm.eid <<  ">" << endl;
}

void printEntity(const Entity& e, SubEntityMap& subEntityMap, const MentionVector& ml) {
    cout << endl<< "printEntity:"<<"entity id=" <<  e.eid << endl <<"[" <<endl;
    for(auto it = e.subEntities.begin(); it != e.subEntities.end(); it++) {
        for(auto menIter = subEntityMap[*it].sub_mentions.begin();menIter!=subEntityMap[*it].sub_mentions.end();menIter++)
        printMention( ml[*menIter]);
    }
    cout << "]" << endl<<endl;
}


/** Task, in this case a pair of mentions or entities that should be merged

    NOTE: Since we have as many entities as we have mentions, the
    numers can be interpreted in any way the state wants.
 */
class Task {
    public:
        int src_eid;
        int des_eid;

    public:
        Task():src_eid(-1), des_eid(-1) {}
        Task(int srcid, int desid):src_eid(srcid), des_eid(desid) {}
};

class Coreference;

class LocalScheduler {
    int numToGenerate; // how many tasks we still need to generate?
    Coreference* coref;
public:
    LocalScheduler(int num_threads, Coreference* coref);

    size_t find_des_eid(size_t src_eid); 

    bool GetNextTask(Task& task);
};

/* convergence GLA */
class CGLA {
    int numIter;
public:
    CGLA(int _numIter):numIter(_numIter) {}

    // not needed void AddItem(int){}
    void AddState(CGLA& other) {}

    bool ShouldIterate(void) {
        return numIter < num_iter;
    }
};

/* State object for coreference */
class Coreference {
    int cur_iter;

public:
    typedef std::pair<LocalScheduler*, CGLA*> WorkUnit;
    typedef std::vector<WorkUnit> WUVector;
    MentionVector mentions;
    SubEntityMap subEntities;
    EntityVec entities;
    SuperEntityVec superEntities;
public:

    // Constructor
    Coreference( const MentionsGLA& mentionsGLA ) {
        cout << "Building the GIST from the GLA" << endl;
        MentionsGLA& mentionsGLA2 = const_cast<MentionsGLA&>(mentionsGLA); // nasty but copying happens otherwise
        mentionsGLA2.GetMentionsBySwapping(mentions);
        mentionsGLA2.GetSubEntitiesBySwapping(subEntities);
        mentionsGLA2.GetEntitiesBySwapping(entities);
        mentionsGLA2.GetSuperEntitiesBySwapping(superEntities);
          
        /*for(auto it=mentions.begin(); it!=mentions.end(); it++)
            cout<<it->mid<<" "<<it->tokenVec.size()<<endl;      
  
           for(auto it=entities.begin(); it!=entities.end();it++){
           cout<<"entity="<<it->second.eid<<" ";
           for(auto it2=it->second.subEntities[0].sub_mentions.begin(); it2!=it->second.subEntities[0].sub_mentions.end(); it2++)
             cout<<*it2<<" ";
           cout<<endl;
        }*/
        
        /*for(auto it=superEntities.begin();it!=superEntities.end();it++)
         {
           cout<<it->second.seid<<" ";
           for(auto it2=it->second.entities.begin(); it2!=it->second.entities.end(); it2++)
            cout<<*it2<<" ";
           cout<<endl;
        }*/       

        cur_iter = 0;
    }

    // Destructor
    ~Coreference() {}

    void PrepareRound( WUVector& workUnits, size_t paraHint ) {
        ++cur_iter;
        cout<<"cur_iter="<<cur_iter<<endl;
        //paraHint = paraHint*2;   
        //paraHint = 1;
        for(int i=0; i<paraHint; i++) {
            CGLA* cgla = new CGLA(cur_iter);
            LocalScheduler* ls = new LocalScheduler(paraHint, this);
            workUnits.push_back(make_pair(ls,cgla));
        }
    }

    void DoStep(Task& task, CGLA& cGla) {
        int src_eid = task.src_eid;
        int des_eid = task.des_eid;
        int large=-1;
        int small=-1;

        assert(src_eid != des_eid);
        if(src_eid>des_eid) {
            large = src_eid;
            small = des_eid;
        } else {
            large = des_eid;
            small = src_eid;
        }

        double loss=0;
        double gain=0;
        bool accept = false;
        entities[small].lock();
        entities[large].lock();       
        if(entities[src_eid].subEntities.size()!=0){
            int src_sub_eid_index = RandInt()%entities[src_eid].subEntities.size();

            for(int i=0; i<entities[src_eid].subEntities.size(); i++){
                if(i == src_sub_eid_index) continue;
                loss+=subEntities[entities[src_eid].subEntities[src_sub_eid_index]].pairwiseScore(
                      subEntities[entities[src_eid].subEntities[i]], mentions);
            }

            if(loss!=0)
                loss=loss/(entities[src_eid].subEntities.size()-1); 

            for(int i=0; i!=entities[des_eid].subEntities.size(); i++){
                gain+=subEntities[entities[src_eid].subEntities[src_sub_eid_index]].pairwiseScore(
                      subEntities[entities[des_eid].subEntities[i]], mentions);
            }
            if(entities[des_eid].subEntities.size()!=0)
                gain=gain/(entities[des_eid].subEntities.size());

            //cout<<"gain="<<gain<<" loss="<<loss<<endl;
            if(loss==0){ 
                if(gain>1/sqrt(2))  // we should accept it
                    accept=true;
            } else{
                if(gain-loss>0)
                    accept=true;
            }
            //else {// accept it with a probablity
            //    double ratio=exp(gain-loss);
            //    double p = RandDouble();
            //if(ratio>p) accept=true;
            //}
            if(accept) {
               // cout<<"accept"<<endl;
                //cout<<"gain="<<gain<<" loss="<<loss<<endl<<"before inserting src"<<endl;
                //printEntity(entities[src_eid], subEntities, mentions);
                //printEntity(entities[des_eid], subEntities, mentions);
                entities[des_eid].insert(src_sub_eid_index, entities[src_eid].subEntities[src_sub_eid_index], mentions, subEntities, superEntities);
                entities[src_eid].remove(src_sub_eid_index, entities[src_eid].subEntities[src_sub_eid_index], mentions, subEntities, superEntities);
                //cout<<"end inserting"<<endl;
                //printEntity(entities[des_eid], subEntities, mentions);
            }
        }
        entities[large].unlock();
        entities[small].unlock();
    }

private:
    int cur_mention;
    
public:
    void Finalize() {
         cout << "I'm in Finalize" << endl;
         cur_mention =0;
    }

    bool GetNextResult(int& men_id, int& ent_id) {
        assert(cur_iter <= num_iter); 
        if(cur_iter < num_iter)
            return false;
        else if(cur_iter == num_iter){
            if (cur_mention == mentions.size())
                return false;
            else {
                men_id=mentions[cur_mention].mid;
                ent_id=mentions[cur_mention].eid;
                cur_mention++;
            }
        }  
        return true;
    }
};


LocalScheduler::LocalScheduler(int num_threads, Coreference* coref){
    this->numToGenerate = coref->mentions.size()/num_threads;
    //this->numToGenerate = 2000;
    this->coref=coref;
}

size_t LocalScheduler::find_des_eid(size_t src_eid) {
    int super_eid = -1;
    int des_eid = -1;
    coref->entities[src_eid].lock();
    if(coref->entities[src_eid].superEntities.size()!=0){
        /*cout<<"src_eid<<"<<src_eid<<" ";
        for(auto it2=coref->entities[src_eid].superEntities.begin();it2!=coref->entities[src_eid].superEntities.end();it2++)
           cout<<"super eid="<<*it2<<" ";
        cout<<endl;*/
        super_eid = coref->entities[src_eid].superEntities[RandInt()%coref->entities[src_eid].superEntities.size()];
    }
    coref->entities[src_eid].unlock(); 

    if(super_eid!=-1){
        int hash_index = RandInt()%coref->superEntities[super_eid].num_segs;
        coref->superEntities[super_eid].lock(hash_index);
        if(coref->superEntities[super_eid].hash_entities[hash_index].size()!=0){
            int random_des_eid_index = RandInt()%coref->superEntities[super_eid].hash_entities[hash_index].size();
            auto it=coref->superEntities[super_eid].hash_entities[hash_index].begin();
            for(int i=0; i<random_des_eid_index; i++) it++;
            des_eid = *it;
        }
        coref->superEntities[super_eid].unlock(hash_index);
    }
    return des_eid;
}

bool LocalScheduler:: GetNextTask(Task& task) {
    if (numToGenerate<=0)
        return false;
    else {
        numToGenerate--;
        while(true){
            task.src_eid=-1;
            task.des_eid=-1;
            while(task.src_eid==-1 || coref->entities[task.src_eid].subEntities.size()==0){ 
                task.src_eid = RandInt()%(coref->entities.size());
            }
            task.des_eid = find_des_eid(task.src_eid);
            if(task.des_eid!=-1 && task.des_eid!=task.src_eid){
                //cout<<"src_eid=="<<task.src_eid<<"des_eid=="<<task.des_eid<<endl;
                return true;
            }
        }
    }
}

#endif //  COREFERENCE_H_
