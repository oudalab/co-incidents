import json
import pickle
import gensim
import os
import re
from nltk.tokenize import RegexpTokenizer
from stop_words import get_stop_words
from nltk.stem.porter import PorterStemmer
from gensim.models.doc2vec import TaggedDocument


from sklearn.feature_extraction.text import CountVectorizer
def bow_extractor(corpus,ngram_range=(1,1)):
    vectorizer=CountVectorizer(min_df=5,max_df=100000,ngram_range=ngram_range)#,max_features=100)
    features=vectorizer.fit_transform(corpus)
    return vectorizer,features

#only take the first 10000 to train the doc2vec model.
with open('datawithdoc.txt','r') as infile:
    docs=json.load(infile)
CORPUS=[]
count=0
for doc in docs:
    CORPUS.append(doc["doc"])

bow_vectorizer,bow_features=bow_extractor(CORPUS)
#features=bow_features.todense()
dic={}
dic["bow_vectorizer"]=bow_vectorizer
dic["bow_features"]=bow_features
filename = 'bow_vectorizer_all.model'
pickle.dump(bow_vectorizer, open(filename, 'wb'))
filename1 = 'bow_features_all.model'
pickle.dump(bow_features, open(filename1, 'wb'))

