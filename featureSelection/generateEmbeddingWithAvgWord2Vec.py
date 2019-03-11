#!/usr/bin/python
# -*- coding: utf-8 -*-
import gensim
from gensim import utils
import numpy as np
import sys
from sklearn.datasets import fetch_20newsgroups
from nltk import word_tokenize
from nltk import download
from nltk.corpus import stopwords
import matplotlib.pyplot as plt
import json
import pickle
import os
import re
from nltk.tokenize import RegexpTokenizer
from stop_words import get_stop_words
from nltk.stem.porter import PorterStemmer
from gensim.models.doc2vec import TaggedDocument

download('punkt')  # tokenizer, run once
download('stopwords')  # stopwords dictionary, run once
stop_words = stopwords.words('english')

print ('start loading the word2vec pretrained model.')
model = \
    gensim.models.KeyedVectors.load_word2vec_format('GoogleNews-vectors-negative300.bin.gz'
        , binary=True)
print ('end loading the word2vec pretrained model. ')


def preprocess(text):
    text = text.lower()
    doc = word_tokenize(text)
    doc = [word for word in doc if word not in stop_words]
    doc = [word for word in doc if word.isalpha()]  # restricts string to alphabetic characters only
    return doc


def document_vector(word2vec_model, doc):

    # remove out-of-vocabulary words

    doc = [word for word in doc if word in word2vec_model.vocab]
    return np.mean(word2vec_model[doc], axis=0)


count = 0
totalcount = 0
dataWithVec = []
index = 0
with open('/home/yan/hanover_backup/coincidenceData/DallasData/merged6.json'
          , 'r') as infile:
    for line in infile:
        if totalcount > 20000:
            break
        else:
            if totalcount % 20000 == 0:
                print ('{} processed'.format(totalcount))
            doc = json.loads(line)
        try:
            processed_doc = preprocess(doc['doc'])
            data = {}
            data['id'] = doc['mongoid']
            data['mediasource1'] = doc['time']
            data['mediasource2'] = doc['source']
            #change the ndarray to an array to be json serializable
            data['embed'] = document_vector(model, processed_doc).flatten()  # hotembedding.todense().tolist()[0]
            data['code'] = doc['code']
            data['date8'] = doc['date8']
            data['day'] = doc['day']
            data['year'] = doc['year']
            data['month'] = doc['month']
            data['goldstein'] = doc['goldstein']
            data['quad_class'] = doc['quad_class']
            data['root_code'] = doc['root_code']
            data['src_actor'] = doc['src_actor']
            data['src_agent'] = doc['src_agent']
            data['tgt_actor'] = doc['tgt_actor']
            data['tgt_agent'] = doc['tgt_agent']
            data['tgt_other_agent'] = doc['tgt_other_agent']
            data['latitude'] = doc['lat']
            data['longitude'] = doc['lon']
            data['geoname'] = doc['location_name']
            totalcount = totalcount + 1
            dataWithVec.append(data)
            if totalcount % 1000 == 0:
                with open('/home/yan/hanover_backup/coincidenceData/DallasData/datawithembed0311-avg.json'
                          , 'a') as outfile:
                    print ('save to disk: ' + str(totalcount))
                    for d in dataWithVec:
                        json.dump(d, outfile)
                        outfile.write('\n')
        except Exception as e:
            count = count + 1
            print (e)
            print ('count:' + str(count) + ' ' + 'totalcount: ' \
                + str(totalcount))


            
