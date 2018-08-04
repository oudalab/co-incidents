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

class StreamArray(list):
    """
    Converts a generator into a list object that can be json serialisable
    while still retaining the iterative nature of a generator.

    IE. It converts it to a list without having to exhaust the generator
    and keep it's contents in memory.
    """
    def __init__(self, generator):
        self.generator = generator
        self._len = 1

    def __iter__(self):
        self._len = 0
        for item in self.generator:
            yield item
            self._len += 1

    def __len__(self):
        """
        Json parser looks for a this method to confirm whether or not it can
        be parsed
        """
        return self._len


def large_list_generator_func():
    totalcount=0;
    bow_vectorizer=pickle.load(open("bow_vectorizer_150_new.model","rb"));
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
              data={};
             break;
          doc=json.loads(line)
          try:
              doctemp=[]
              doctemp.append(doc["doc"])
              hotembedding=bow_vectorizer.transform(doctemp)
              data={};
              data["id"]=doc["mongoid"]
              data["mediasource1"]=doc["from"]
              data["mediasource2"]=doc["source"]
              data["embed"]=hotembedding.todense().tolist()[0]
              data["code"]=doc["code"]
              data['date8']=doc['date8']
              data['day']=doc['day']
              data['year']=doc['year']
              data['month']=doc['month']
              data['goldstein']=doc['goldstein']
              data['quad_class']=doc['quad_class']
              data['root_code']=doc['root_code']
              data['src_actor']=doc['src_actor']
              data['src_agent']=doc['src_agent']
              data['src_other_agent']=doc['src_othere_agent']
              data['target']=doc['target']
              data['tgt_actor']=doc['tgt_actor']
              data['tgt_agent']=doc['tgt_agent']
              data['tgt_other_agent']=doc['tgt_other_agent']
              #dataWithVec.append(data)
              yield(data)
          except Exception as e:
                #count=count+1;
                #print("count:"+str(count)+" "+"totalcount: "+str(totalcount));
               print(e)

def main():
    #database = "./events.db"
    # create a database connection
    #conn = create_connection(database)
    bow_vectorizer=pickle.load(open("bow_vectorizer_150_new.model","rb"));
   # dataWithVec=[]
   # totalcount=0
    large_generator_handle = large_list_generator_func()
    stream_array = StreamArray(large_generator_handle)

    with open('nudup-embed.json', 'w') as fp:
       for chunk in json.JSONEncoder().iterencode(stream_array):
        #print 'Writing chunk: ', chunk
        fp.write(chunk)
if __name__ == '__main__':
    main()


