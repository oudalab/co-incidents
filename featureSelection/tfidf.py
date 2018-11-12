import json
import pickle
import os
import re
#import gensim
import gensim
from nltk.tokenize import RegexpTokenizer
from stop_words import get_stop_words
from nltk.stem.porter import PorterStemmer
from gensim.models.doc2vec import TaggedDocument


from sklearn.feature_extraction.text import CountVectorizer
def bow_extractor(corpus,ngram_range=(1,1)):
    vectorizer=CountVectorizer(min_df=0.01,max_df=0.95,stop_words="english",ngram_range=ngram_range,max_features=100)
    vectorizer.fit_transform(corpus)
    return vectorizer

#only take the first 10000 to train the doc2vec model.
print("start loading")
CORPUS=[]
count=0
#it has 26G data--1/4 data of the original dataset that I have.
with open('./merged6.json','r') as infile:
    #docs=json.load(infile)
    ##huge file iteratively load it not all load it once
    for line in infile:
        if(count<1000000):
            data=json.loads(line);
            CORPUS.append(data["doc"])
            count=count+1
#print("end loading");


#for doc in docs:
#    count=count+1
#    if(count<15489981):
#        CORPUS.append(doc["doc"])
#        if(doc["doc"]==""):
#           print(doc["doc"])
print("done loading the docs and now startt to train")
bow_vectorizer=bow_extractor(CORPUS)
#features=bow_features.todense()
#dic={}
#dic["bow_vectorizer"]=bow_vectorizer
#dic["bow_features"]=bow_features
filename = 'bow_vectorizer_100_new.model'
pickle.dump(bow_vectorizer, open(filename, 'wb'))
#filename1 = 'bow_features_75_new.model'
#pickle.dump(bow_features, open(filename1, 'wb'))



