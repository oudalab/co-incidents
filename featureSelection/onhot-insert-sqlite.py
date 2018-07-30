import sqlite3
import json
import pickle
import gensim
import os
import re
from sqlite3 import Error
from nltk.tokenize import RegexpTokenizer
from stop_words import get_stop_words
from nltk.stem.porter import PorterStemmer
from gensim.models.doc2vec import TaggedDocument
from sklearn.feature_extraction.text import TfidfTransformer
import numpy as np
 
def create_connection(db_file):
    """ create a database connection to a SQLite database """
    try:
        conn = sqlite3.connect(db_file)
        print(sqlite3.version)
        return conn
    except Error as e:
        print(e)
    return None

def create_event(conn, event):
    """
    Create a new project into the projects table
    :param conn:
    :param project:
    :return: project id
    """
    try:
	    sql = ''' INSERT INTO events(embed,tgt_actor,root_code,src_actor,mediasource2,target,goldstein,tgt_other_agent,code,day,month,quad_class,mediasource1,src_other_agent,id,tgt_agent,date8,year)
	              VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?) '''
	    cur = conn.cursor()
	    cur.execute(sql, event)
    except Exception as e:
        print(e)
    #return cur.lastrowid

def tfidf_transformer(bow_matrix):
    transformer=TfidfTransformer(norm='l2',smooth_idf=True,use_idf=True)
    tfidf_matrix=transformer.fit_transform(bow_matrix)
    return transformer,tfidf_matrix

def main():
    database = "./events.db"
    # create a database connection
    conn = create_connection(database)
    bow_vectorizer=pickle.load(open("bow_vectorizer_150_new.model","rb"));
    with conn:
        count=0;
        totalcount=0;
        with open('./nodup.json','r') as infile:
            for line in infile:
                totalcount=totalcount+1
                if(totalcount%20000==0):
                	print(str(totalcount)+" processed")
                if totalcount>59489980:
                	break;
                doc=json.loads(line)
                try:
                    doctemp=[]
                    doctemp.append(doc["doc"])
                    hotembedding=bow_vectorizer.transform(doctemp)
                     # data={};			
                     # data["id"]=doc["mongoid"]
                     # data["mediasource1"]=doc["from"]
                     # data["mediasource2"]=doc["source"]
                     # data["embed"]=hotembedding.todense().tolist()[0]
                     # data["code"]=doc["code"]
                     # data['date8']=doc['date8']
                     # data['day']=doc['day']
                     # data['year']=doc['year']
                     # data['month']=doc['month']
                     # data['goldstein']=doc['goldstein']
                     # data['quad_class']=doc['quad_class']
                     # data['root_code']=doc['root_code']
                     # data['src_actor']=doc['src_actor']
                     # data['src_agent']=doc['src_agent']
                     # data['src_other_agent']=doc['src_othere_agent']
                     # data['target']=doc['target']
                     # data['tgt_actor']=doc['tgt_actor']
                     # data['tgt_agent']=doc['tgt_agent']
                     # data['tgt_other_agent']=doc['tgt_other_agent']
                     #dataWithVec.append(data)
                    event=(str(hotembedding.todense().tolist()[0]).strip('[]'),doc['tgt_actor'],doc['root_code'],doc['src_actor'],doc["source"],doc['target'],doc['goldstein'],doc['tgt_other_agent'],doc["code"],doc['day'],doc['month'],doc['quad_class'],doc["from"],doc['src_othere_agent'],doc["mongoid"],doc['tgt_agent'],doc['date8'],doc['year'])	       
                    create_event(conn,event)                 
                except Exception as e:
                  	count=count+1;
                  	print("count:"+str(count)+" "+"totalcount: "+str(totalcount));

if __name__ == '__main__':
    main()
