import json
import pickle
import gensim
import os
import re
import csv
from nltk.tokenize import RegexpTokenizer
from stop_words import get_stop_words
from nltk.stem.porter import PorterStemmer
from gensim.models.doc2vec import TaggedDocument


from sklearn.feature_extraction.text import CountVectorizer
def bow_extractor(corpus,ngram_range=(1,1)):
    vectorizer=CountVectorizer(min_df=0.01,max_df=0.95,stop_words="english",ngram_range=ngram_range,max_features=600)
    features=vectorizer.fit_transform(corpus)
    return vectorizer,features

with open("/home/yan/coincidenceData/terrier-documents.shrunk.3.tsv",'r') as infile:
    docs=csv.reader(infile,delimiter='\t')
    CORPUS=[]
    count=0
    for row in docs:
        CORPUS.append(row[3])

bow_vectorizer,bow_features=bow_extractor(CORPUS)
#features=bow_features.todense()
dic={}
dic["bow_vectorizer"]=bow_vectorizer
dic["bow_features"]=bow_features
filename = '_tsv_bow_vectorizer_600.model'
pickle.dump(bow_vectorizer, open(filename, 'wb'))
filename1 = 'tsv_bow_features_600.model'
pickle.dump(bow_features, open(filename1, 'wb'))
