import json
import pickle
import gensim
import os
import re
from nltk.tokenize import RegexpTokenizer
from stop_words import get_stop_words
from nltk.stem.porter import PorterStemmer
from gensim.models.doc2vec import TaggedDocument



from sklearn.feature_extraction.text import TfidfTransformer
import numpy as np
def tfidf_transformer(bow_matrix):
    transformer=TfidfTransformer(norm='l2',smooth_idf=True,use_idf=True)
    tfidf_matrix=transformer.fit_transform(bow_matrix)
    return transformer,tfidf_matrix

#only take the first 10000 to train the doc2vec model.
bow_vectorizer=pickle.load(open("bow_vectorizer_150_new.model","rb"));
#bow_features=pickle.load(open("bow_features_300_new.model","rb"));
#tfidf_trans,tfidf_features=tfidf_transformer(bow_features)

count=0;
totalcount=0;
dataWithVec=[];
with open('./nodup-1001.json','r') as infile:
        for line in infile:
                if(totalcount%20000==0):
                        print("{} processed".format(totalcount))
                        totalcount=totalcount+1
                        doc=json.loads(line)
                try:
                        doctemp=[]
                        doctemp.append(doc["doc"])
                        hotembedding=bow_vectorizer.transform(doctemp)
                        data={};
                        data["id"]=doc["mongoid"]
                        data["mediasource1"]=doc["from"]
                        data["mediasource2"]=doc["sorce"]
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
                        data['tgt_actor']=doc['tgt_actor']
                        data['tgt_agent']=doc['tgt_agent']
                        data['tgt_other_agent']=doc['tgt_other_agent']
                        data['latitude']=doc['latitude']
                        data["longitude"]=doc["longitude"]
                        data["geoname"]=doc["geoname"]
                        dataWithVec.append(data)
                except Exception as e:
                        count=count+1;
                        print(e)
                        print("count:"+str(count)+" "+"totalcount: "+str(totalcount));
with open('data-150VOC-1001.json','w') as outfile:
        json.dump(dataWithVec,outfile);
