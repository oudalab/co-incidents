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
import gzip

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
count_hasdoc=0
file_index = sys.argv[1]
dataWithVec = []
#1975 to 2018 25+18+1=44 add 1 for node year find
for i in range(0,45):
    dataWithVec.append([])

index = 0
with gzip.open('/home/lian9478/OU_Coincidence/dallasData/terrier-location-text-source-part' + file_index + '.json.gz'
          , 'rb') as infile:
    for line in infile:
        try:
            if totalcount%20000 == 0:
                print ('{} processed'.format(totalcount))
            year_tosave="";
            doc = json.loads(line.decode('utf-8'))
            processed_doc=""
            if 'text' in doc:
                processed_doc = preprocess(doc['text'])
            data = {}
            data['latitude'] = ""
            data['longitude'] = ""
            data['geoname'] = ""
            data['countrycode'] = ""
            data['statecode'] = ""
            data["stategeonameid"] = ""
            data["countrygeonameid"] = ""
            data['id'] = ""
            data['source'] = ""
            data['code'] = ""
            data['date8'] = ""
            data['day'] = ""
            data['year'] = ""
            data['month'] = ""
            data['target']=""
            data['id'] = ""
            data['source'] = ""
            data['root_code'] = ""
            data['src_actor'] = ""
            data['src_agent'] = ""
            data['tgt_actor'] = ""
            data['tgt_agent'] = ""
            data['tgt_other_agent'] = ""
            data['mongo_id'] = ""
            #change the ndarray to an array to be json serializablei
            if(processed_doc!=""):
                data['embed'] = document_vector(model, processed_doc).tolist()
                count_hasdoc=count_hasdoc+1 # hotembedding.todense().tolist()[0]
            data['code'] = ""
            if 'code' in doc:
                data['code'] = doc['code']
            if 'mongo_id' in doc:
                data['mongo_id'] = doc['mongo_id']
            if 'date8' in doc:
                data['date8'] = doc['date8']
            if 'day' in doc:
                data['day'] = doc['day']
            if 'year' in doc:
                data['year'] = doc['year']
                year_tosave = doc['year']
            else:
                year_tosave=""
            if 'month' in doc:
                data['month'] = doc['month']
            if 'target' in doc:
                data['target']= doc['target']
            if 'root_code' in doc:
                data['root_code'] = doc['root_code']
            if 'scr_actor' in doc:
                data['src_actor'] = doc['src_actor']
            if 'src_agent' in doc:
                data['src_agent'] = doc['src_agent']
            if 'tgt_actor' in doc:
                data['tgt_actor'] = doc['tgt_actor']
            if 'tgt_agent' in doc:
                data['tgt_agent'] = doc['tgt_agent']
            if 'tgt_other_agent' in doc:
                data['tgt_other_agent'] = doc['tgt_other_agent'] 
            if 'geo_location' in doc:
                data['latitude'] = doc['geo_location']['lat']
                data['longitude'] = doc['geo_location']['lon']
                data['geoname'] = doc['geo_location']['location_name']
                data['countrycode'] = doc['geo_location']['countryCode']
                data['statecode'] = doc['geo_location']["stateCode"]
                data["stategeonameid"] = doc['geo_location']["stateGeoNameId"]
                data["countrygeonameid"] = doc['geo_location']["countryGeoNameId"]
            totalcount = totalcount + 1
            if year_tosave != "":
                dataWithVec[int(year_tosave)-1975].append(data)
            else:
                #append the one with no year information goes to the last element in the vec.
                dataWithVec[44].append(data)
            if totalcount % 1000 == 0:
                    year_index= 1974
                    for vec in dataWithVec:
                            year_index = year_index + 1
                            with open('/home/lian9478/OU_Coincidence/dallasData/datawithembed0527_' + str(year_index) + '.json'
                              , 'a') as outfile:
                                #pickle.dump(vec, outfile)
                                for d in vec:
                                    json.dump(d, outfile)
                                    outfile.write('\n')
                            print ('save to disk: ' + str(len(vec)))
                    dataWithVec=[]
                    for i in range(0,45):
                        dataWithVec.append([])
        except Exception as e:
            count = count + 1
            print (e)
            print ('count:' + str(count) + ' ' + 'totalcount: ' \
                + str(totalcount))
print(str(count)+ " of docs has text within "+str(totalcount))        

            
