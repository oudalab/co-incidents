"""
This file presents a simplified SampleRank experiment focused on features.
To minimize complication, we simply generate Sentence objects based on
given data and organize them into Incidents, which are then considered
part of a Configuration. We run through varying configurations to find
the best one, recording feature importance along the way.

Features "experiment" function to vary parameters and observe performance.

@author Elena Montes
"""


import click
import json
import logging
import numpy as np
import matplotlib.pyplot as plt
import re

from copy import deepcopy
from functools import lru_cache
from itertools import combinations
from gensim.models import Doc2Vec
from gensim.models.doc2vec import TaggedDocument
from gensim.utils import to_unicode
from nltk.stem.porter import PorterStemmer
from nltk.tokenize import RegexpTokenizer
from random import choice, random, sample
from stop_words import get_stop_words
from time import time, localtime, asctime
# from gensim.models.keyedvectors import Doc2VecKeyedVectors


class Feature:
    """ Feature, with assoc function to call, weight, and boolean 
        to show if it was last 'on' (evaluated to True) """

    def __init__(self, feature_id, assoc_function):
        self.feature_id = feature_id
        self.assoc_function = assoc_function
        self.weight = 1
        self.on = False


@lru_cache(maxsize=None)
def eq_country_code(group):
    if len(group) > 0:
        group = set(group)
        first = group.pop()
        on = all(first.country_code == item.country_code for item in group)
        return on

    return True


@lru_cache(maxsize=None)
def eq_src_actor(group):
    if len(group) > 0:
        group = set(group)
        first = group.pop()
        on = all(first.src_actor == item.src_actor for item in group)
        return on

    return True


@lru_cache(maxsize=None)
def eq_src_agent(group):
    if len(group) > 0:
        group = set(group)
        first = group.pop()
        on = all(first.src_agent == item.src_agent for item in group)
        return on

    return True


@lru_cache(maxsize=None)
def eq_tgt_actor(group):
    if len(group) > 0:
        group = set(group)
        first = group.pop()
        on = all(first.tgt_actor == item.tgt_actor for item in group)
        return on

    return True


@lru_cache(maxsize=None)
def eq_tgt_agent(group):
    if len(group) > 0:
        group = set(group)
        first = group.pop()
        on = all(first.tgt_agent == item.tgt_agent for item in group)
        return on

    return True


@lru_cache(maxsize=None)
def eq_geoname(group):
    if len(group) > 0:
        group = set(group)
        first = group.pop()
        on = all(first.geoname == item.geoname for item in group)
        return on

    return True


class Sentence:
    """ 
    One sentence, with certain attributes attached. 
    """

    def __init__(self, id_num, code, src_actor, src_agent, src_other_agent, tgt_actor, tgt_agent,
                 tgt_other_agent, country_code, geoname, latitude, longitude):
        """ 
        Initialize this sentence with specified attributes. 
        """
        self.id_num = id_num
        self.inc_id = None

        self.code = code
        self.src_actor = src_actor
        self.src_agent = src_agent
        self.src_other_agent = src_other_agent
        self.tgt_actor = tgt_actor
        self.tgt_agent = tgt_agent
        self.tgt_other_agent = tgt_other_agent
        self.country_code = country_code
        self.geoname = geoname
        self.latitude = latitude
        self.longitude = longitude

        self.pair_ids = list()

    def __str__(self):
        """ 
        :return: string representation of this sentence. 
        """
        return self.id_num


class Incident:
    """
    One clustering of Sentences.
    """

    def __init__(self, num_inc_features, num_pair_features):
        """
        Initialize this Incident.
        :param num_inc_features: number of incident features to be used for scoring
        :param num_pair_features: number of pairwise features to be used for scoring
        """
        self.id = id(self)
        self.features_on = np.zeros((num_inc_features,), dtype=int)
        self.sentences = dict()
        self.pairs = list()

        # Remember for purpose of initializing Pair objects
        self.num_pair_features = num_pair_features

    def add_sentence(self, sentence):
        """
        Add a sentence to this incident. Generate relevant pairs.
        :param sentence: sentence object to be added
        :return: nothing
        """

        # Add all relevant pairs
        for other in self.sentences.values():

            # Create new pair object
            pair = (sentence, other)

            # Save to this incident
            self.pairs.append(pair)

        # Add this sentences
        self.sentences[sentence.id_num] = sentence
        sentence.inc_id = self.id

    def calculate_score(self, inc_features, pair_features):
        """
        Calculate total score for this cluster based on given features. 
        Records which features turned on.
        :param inc_features: incident features
        :param pair_features: pairwise features
        :return: total score as a float
        """
        # Total score for this sub-incident
        total = 0.0

        # Evaluate all relevant features for this sub incident
        for i, feature in enumerate(inc_features):
            on = feature.assoc_function(frozenset(self.get_sentences()))
            total += on * feature.weight

            # Record result of feature
            self.features_on[i] = on

        # Evaluate pairwise feature scores
        for pair in self.pairs:
            for feature in pair_features:
                total += feature.assoc_function(frozenset(pair))

        return total

    def get_sentences(self):
        """
        Method to return all sentences contained within this incident. 
        :return: list of sentences
        """
        return list(self.sentences.values())

    def remove_sentence(self, sent_id):
        """
        Remove sentence with given id from this incident cluster.
        :param sent_id: id of a sentence object
        :return: sentence object corresponding to given id
        """

        # Delete all associated pairs
        self.pairs[:] = [pair for pair in self.pairs if not (pair[0].id_num == sent_id or pair[1].id_num == sent_id)]

        # Remove and return sentence object
        return self.sentences.pop(sent_id)


class Configuration:
    """ 
    One configuration of super-incidents. 
    """

    def __init__(self, incidents):
        """ 
        Initialize this configuration with specified attributes. 
        :param incidents: dictionary of incident objects (keys: ids)
        """
        self.id = id(self)
        self.incidents = incidents

    def __str__(self):
        """ 
        :return: string representation of this configuration. 
        """
        return 'configuration {}: \n'.format(self.id) + '\n\n'.join([str(incident)
                                                                    for incident in self.incidents.values()])

    def get_sentences(self):
        """ 
        :return: list of all sentences within this configuration.
        """
        return list(sum(list(incident.get_sentences() for incident in self.incidents.values()), []))

    def calculate_score(self, inc_features, pair_features):
        """
        Calculate score based on all incident and in-incident pair 
        feature scores in this configuration.
        :param inc_features: incident-wide features
        :param pair_features: pairwise features
        :return: a score as a float value
        """
        total = 0.0

        # Calculate total score of all incidents
        for incident in self.incidents.values():
            total += incident.calculate_score(inc_features, pair_features)

        return total

    def calculate_feat_updates(self, prev_updates, prev_config, num_inc_features):
        """
        Determine which incident features were turned on and off from previous config. 
        Add to running sum of feature weight updates.
        :param prev_updates: total thus far of feature weight updates
        :param prev_config: a configuration object representing previous setup
        :param num_inc_features: number of incident features
        :return: new list of weight running updates
        """

        # Sum of all differences (turned on and off features)
        diff = np.zeros((num_inc_features,), dtype=int)

        for incident in self.incidents:

            # Grab on/off status of features in this config
            new_feats = self.incidents[incident].features_on

            # Grab matching cluster's on/off features
            if incident not in prev_config.incidents:
                prev_feats = np.zeros((num_inc_features,), dtype=int)
            else:
                prev_feats = prev_config.incidents[incident].features_on

            # Calculate difference and save to total difference
            diff = np.add(diff, np.subtract(new_feats, prev_feats))

        # Save total updates to dictionary of previous updates
        new_updates = np.add(prev_updates, diff)

        return new_updates

    def calculate_accuracy(self, labeled_samples):
        num_correct = 0
        total = 0
        for incident in self.incidents:
            # Get all labeled points in this incident cluster
            data = [s for s in incident.sentences.keys() if s in labeled_samples]
            labels = dict()

            # Generate counts for all labels seen
            for d in data:
                label = labeled_samples[d]
                if label in labels:
                    labels[label] += 1
                else:
                    labels[label] = 1

            # Mark majority label
            majority_label = max(labels, key=labels.get)

            # Calculate number correctly assigned data points in this cluster
            num_correct += len([s for s in data if labeled_samples[s] == majority_label])

            # Running total of labeled points
            total += len(data)

        # Return purity for all clusters
        return num_correct/total


def gen_sentences(filename):
    """
    Generate Sentence objects for all mentions in the file.
    :param filename: name of file with data
    :return: list of all Sentence objects
    """
    sentences = []
    docs = dict()

    logging.info('Loading json file and generating sentences.')

    # Open up json and iterate through
    doc = json.load(open(filename, 'r'))

    for elt in doc:
        # Get relevant attributes
        id_num = elt['id'] if 'id' in elt else None
        code = elt['code'] if 'code' in elt else None
        src_actor = elt['src_actor'] if 'src_actor' in elt else None
        src_agent = elt['src_agent'] if 'src_agent' in elt else None
        src_other_agent = elt['src_other_agent'] if 'src_other_agent' in elt else None
        tgt_actor = elt['tgt_actor'] if 'tgt_actor' in elt else None
        tgt_agent = elt['tgt_agent'] if 'tgt_agent' in elt else None
        tgt_other_agent = elt['tgt_other_agent'] if 'tgt_other_agent' in elt else None
        country_code = elt['country_code'] if 'country_code' in elt else None
        geoname = elt['geoname'] if 'geoname' in elt else None
        latitude = elt['latitude'] if 'latitude' in elt else None
        longitude = elt['longitude'] if 'longitude' in elt else None

        # Create a sentence
        sentence = Sentence(id_num, code, src_actor, src_agent, src_other_agent, tgt_actor, tgt_agent,
                            tgt_other_agent, country_code, geoname, latitude, longitude)
        sentences.append(sentence)

        docs[id_num] = elt['doc']

    return sentences, docs


def gen_starting_config(is_clustered, sentences, num_inc_features, num_pair_features):
    """
    Generate a starting configuration. It will either cluster all sentences in one
    incident or place them all in their own individual incidents to start.
    :param is_clustered: are the sentences all in one?
    :param sentences: list of all sentences
    :param num_inc_features: number of incident features
    :param num_pair_features: number of pairwise features
    :return: a configuration object
    """

    incidents = dict()

    if is_clustered:  # We will group all sentences in one cluster
        incident = Incident(num_inc_features, num_pair_features)
        for sentence in sentences:
            incident.add_sentence(sentence)
        incidents[incident.id] = incident

    else:  # We will place each sentence in its own cluster
        for sentence in sentences:
            incident = Incident(num_inc_features, num_pair_features)
            incident.add_sentence(sentence)
            incidents[incident.id] = incident

    config = Configuration(incidents)

    return config


def gen_config_update(current_config, prob_isolation, num_inc_features, num_pair_features):
    """
    Generate a potential new configuration. Chance of isolation tells us if the 
    randomly chosen sentence will be put into a brand new incident by itself or
    simply moved between incidents.
    :param current_config: the previous configuration
    :param prob_isolation: chance of moving or isolating
    :param num_inc_features: number of incident features
    :param num_pair_features: number of pairwise features
    :return: new configuration object
    """
    # Choose sentence to move
    sentence = choice(current_config.get_sentences())
    old_inc_id = sentence.inc_id

    # Create new configuration
    new_config = deepcopy(current_config)

    # Record events in case this config is accepted
    log = ''

    if random() <= prob_isolation:
        log += 'Isolated sentence {}\n'.format(sentence.id_num)

        # We will create a new incident with just this sentence
        new_incident = Incident(num_inc_features, num_pair_features)
        new_config.incidents[new_incident.id] = new_incident
    else:
        # Choose where to move this sentence
        new_incident = choice(list(new_config.incidents.values()))
        log += 'Moved sentence {} to incident {}\n'.format(sentence.id_num, new_incident.id)

    # Remove sentence from old incident and add to new incident
    old_incident = new_config.incidents[old_inc_id]
    new_sentence = old_incident.remove_sentence(sentence.id_num)
    new_sentence.inc_id = new_incident.id
    new_incident.add_sentence(new_sentence)

    # Scrap old incident if it's now empty
    if len(old_incident.get_sentences()) == 0:
        del new_config.incidents[old_inc_id]
        log += 'Incident {} was emptied and deleted\n'.format(old_inc_id)

        print(len(new_config.incidents))

    return new_config, log


def printProgressBar(iteration, total, prefix = '', suffix = '', decimals = 1, length = 100, fill = '█'):
    """
    Call in a loop to create terminal progress bar
    @params:
        iteration   - Required  : current iteration (Int)
        total       - Required  : total iterations (Int)
        prefix      - Optional  : prefix string (Str)
        suffix      - Optional  : suffix string (Str)
        decimals    - Optional  : positive number of decimals in percent complete (Int)
        length      - Optional  : character length of bar (Int)
        fill        - Optional  : bar fill character (Str)
    """
    percent = ("{0:." + str(decimals) + "f}").format(100 * (iteration / float(total)))
    filledLength = int(length * iteration // total)
    bar = fill * filledLength + '-' * (length - filledLength)
    print('\r%s |%s| %s%% %s' % (prefix, bar, percent, suffix), end = '\r')
    # Print New Line on Complete
    if iteration == total:
        print()


def train_doc2vec_model(data_filename, num_docs):

    # Necessary NLP tools
    tokenizer = RegexpTokenizer(r'\w+')
    en_stop_words = get_stop_words('en')
    p_stemmer = PorterStemmer()

    tagged_docs = list()
    texts = list()

    # Load in training data file
    with open(data_filename, 'r') as infile:
        raw_file = json.load(infile)

    raw_docs = [{'doc': piece['doc'], 'id': piece['id']} for piece in raw_file]
    raw_docs = sample(raw_docs, num_docs)
    l = len(raw_docs)

    printProgressBar(0, l, prefix='Progress:', suffix='Complete', length=50)

    # Tokenize, stem all documents
    for index, raw in enumerate(raw_docs):
        logging.basicConfig(level=logging.INFO)

        # Clean and tokenize document string
        raw_doc = raw['doc'].lower()
        tokens = tokenizer.tokenize(raw_doc)

        # Remove stop words from tokens
        tokens = [token for token in tokens if token not in en_stop_words]

        # Remove and stem number tokens
        number_tokens = [re.sub(r'[\d]', ' ', token) for token in tokens]
        number_tokens = ' '.join(number_tokens).split()
        stemmed_tokens = [p_stemmer.stem(token) for token in number_tokens]

        # Remove empty tokens
        valid_tokens = [token for token in stemmed_tokens if len(token) > 1]

        # Add tokens to list
        texts.append(valid_tokens)

        # Save tagged document
        tagged_doc = TaggedDocument(to_unicode(' '.join(stemmed_tokens)).split(), [raw['id']])
        tagged_docs.append(tagged_doc)

        printProgressBar(index + 1, l, prefix = 'Progress:', suffix = 'Complete', length = 50)

    # Create and train Doc2Vec model
    model = Doc2Vec(tagged_docs, dm=0, size=len(tagged_docs), alpha=0.025, min_alpha=0.025, min_count=0, workers=8)

    logging.info('Training.')
    model.train(tagged_docs, total_examples=len(tagged_docs), epochs=10)

    return model


def tokenize(doc):
    tokenizer = RegexpTokenizer(r'\w+')
    en_stop_words = get_stop_words('en')
    p_stemmer = PorterStemmer()

    # Clean and tokenize document string
    raw_doc = doc.lower()
    tokens = tokenizer.tokenize(raw_doc)

    # Remove stop words from tokens
    tokens = [token for token in tokens if token not in en_stop_words]

    # Remove and stem number tokens
    number_tokens = [re.sub(r'[\d]', ' ', token) for token in tokens]
    number_tokens = ' '.join(number_tokens).split()
    stemmed_tokens = [p_stemmer.stem(token) for token in number_tokens]

    # Remove empty tokens
    valid_tokens = [token for token in stemmed_tokens if len(token) > 1]

    return valid_tokens


'''def score_final_config(config, model, docs):
    """
    Score the final configuration using random sampled Doc2Vec similarities. 
    :param config: the config to be scored
    :return: final score
    """

    valid_incs = [incident for incident in config.incidents.values() if len(incident.sentences) > 5]
    for inc in valid_incs:
        print(len(inc.sentences))
    #incs = sample([incident for incident in config.incidents.values() if len(incident.sentences) > 5], 2)
    #print(len(incs[0].sentences))
    #print(len(incs[1].sentences))
    #samples = sample(incs[0].sentences.keys(), 5)
    #samples2 = sample(incs[1].sentences.keys(), 5)

    sims = list()

    keyed = Doc2VecKeyedVectors(model.vector_size, 'doc2vec_model')

    # Calculate similarity within sample
    for pair in combinations(samples, 2):
        tokens1 = tokenize(docs[pair[0]])
        tokens2 = tokenize(docs[pair[1]])
        sims.append(keyed.similarity_unseen_docs(model, tokens1, tokens2))

    return sum(sims) / len(sims)

    return 0, 0'''


@click.command()
@click.option('--filename', default='datawithdoc.txt', help='Name of file with data')
@click.option('--is_clustered', default=False, help='Start all sentences together or separate')
@click.option('--num_iterations', default=10000, help='Number of iterations to run')
@click.option('--prob_isolation', default=0.4,
              help='prob [0.0, 1.0] that an incident will be moved to its own new incident')
def experiment(filename, is_clustered, num_iterations, prob_isolation):
    logging.basicConfig(filename='run_{}.log'.format(asctime(localtime(time())).replace(' ', '')),
                        level=logging.INFO)

    pair_eql_country = Feature('ecc', eq_country_code)
    inc_eql_country = Feature('ecc', eq_country_code)

    pair_eql_src_ac = Feature('esac', eq_src_actor)
    inc_eql_src_ac = Feature('esac', eq_src_actor)

    pair_eql_src_ag = Feature('esag', eq_src_agent)
    inc_eql_src_ag = Feature('esag', eq_src_agent)

    pair_eql_tgt_ac = Feature('etac', eq_tgt_actor)
    inc_eql_tgt_ac = Feature('etac', eq_tgt_actor)

    pair_eql_tgt_ag = Feature('etag', eq_tgt_agent)
    inc_eql_tgt_ag = Feature('etag', eq_tgt_agent)

    pair_eql_geo = Feature('eg', eq_geoname)
    inc_eql_geo = Feature('eg', eq_geoname)

    pair_features = [pair_eql_src_ac, pair_eql_src_ag, pair_eql_tgt_ac, pair_eql_tgt_ag, pair_eql_country, pair_eql_geo]
    inc_features = [inc_eql_src_ac, inc_eql_src_ag, inc_eql_tgt_ac, inc_eql_tgt_ag, inc_eql_country, inc_eql_geo]

    feature_updates = np.zeros((len(inc_features),), dtype=int)

    # Generate objects to represent data
    sentences, docs = gen_sentences(filename)
    labeled_docs = dict()
    for actor in {'USA', 'RUS', 'AUS', 'JPN', 'MEX', 'KEN', 'BRA', 'CAN', 'ZAF', 'CHN'}:
        actor_docs = [s for s in sentences if s.src_actor == actor][:50]
        doc_updates = {se.id_num: se for se in actor_docs}
        labeled_docs.update(doc_updates)
    # sentences = sentences[:1000]
    logging.info('COMPLETE: {} sentences generated.\n'.format(len(sentences)))

    # Create a random starting configuration
    current_config = gen_starting_config(is_clustered, sentences, len(inc_features), len(pair_features))
    logging.info('COMPLETE: random configuration created.\n')

    # Score this configuration
    current_score = current_config.calculate_score(inc_features, pair_features)
    logging.info('COMPLETE: calculated initial score of {}.\n'.format(current_score))

    # Scores to plot and show convergence
    scores = []
    iterations = []

    # Doc2Vec model
    doc2vec = Doc2Vec.load('doc2vec_model')

    times_unaccepted = 0

    for iter_num in range(num_iterations):

        logging.info('Beginning iteration {}.\n'.format(iter_num))

        # Create a new trial configuration
        new_config, log = gen_config_update(current_config, prob_isolation, len(inc_features), len(pair_features))
        new_score = new_config.calculate_score(inc_features, pair_features)
        score_diff = new_score - current_score

        # Metropolis Hastings probability of acceptance
        mh_acceptance = new_score / current_score
        print('MH acceptance: {}'.format(mh_acceptance))

        # Acceptance test: score diff is positive or is chosen to randomly change
        if random() <= min(1.0, mh_acceptance):

            times_unaccepted = 0

            logging.info('New configuration accepted. Score update: {}'.format(score_diff))
            logging.info(log)

            # Record changes to feature weights
            feature_updates = current_config.calculate_feat_updates(feature_updates, new_config, len(inc_features))

            # Update configuration
            del current_config
            current_config = new_config
            current_score = new_score

            # Record to plot later
            scores.append(current_score)
            iterations.append(iter_num)

            # Total weight update for all features
            total_weight = sum(feature_updates)

            # Is it time to update feature weights?
            if False:
                for i, feat in enumerate(inc_features):
                    feat.weight += feature_updates[i] / total_weight if total_weight != 0 else 0
                    feature_updates[i] = 0  # reset running weight update sum
        else:
            times_unaccepted += 1

    for i, feat in enumerate(inc_features):
        print('Feature: {} Weight: {}'.format(feat.feature_id, feature_updates[i]))

    result = score_final_config(current_config, doc2vec, docs)
    print('\nFinal config similarity score is {}'.format(result))

    current_config.calculate_accuracy(labeled_samples)

    # plot results
    plt.plot(iterations, scores)
    plt.xlabel('Iteration')
    plt.ylabel('Configuration Score')
    plt.show()
    plt.savefig('run_{}_scores.png'.format(asctime(localtime(time())).replace(' ', '')), bbox_inches='tight')


if __name__ == '__main__':
    experiment()
