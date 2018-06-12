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
bow_vectorizer=pickle.load(open("bow_vectorizer_10000.model","rb"));
bow_features=pickle.load(open("bow_features_10000.model","rb"));
#tfidf_trans,tfidf_features=tfidf_transformer(bow_features)

dataWithVec=[];
with open('datawithdoc.txt','r') as infile:
    docs=json.load(infile)
#this is not tfidf just bow model using count.
count=0;
totalcount=0;
for doc in docs:
        totalcount=totalcount+1
        try:
                doctemp=[]
                doctemp.append(doc["doc"])
                hotembedding=bow_vectorizer.transform(doctemp)
                #nd_tfidf=tfidf_trans.transform(new_doc_features)
                #nd_features=np.round(nd_tfidf.todense(),2)
                data={};
                data["id"]=doc["id"]
                data["embed"]=hotembedding;
                dataWithVec.append(data)
        except:
                #print(str(e))
                count=count+1;
                print("count:"+count+" "+"totalcount: "+totalcount);
filename = 'dataWithEmbedding_tfidf.data'
pickle.dump(dataWithVec, open(filename, 'wb'))
