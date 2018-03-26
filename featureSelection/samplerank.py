from copy import deepcopy
from random import random, choice
import json
from itertools import combinations
import matplotlib.pyplot as plt
from time import time
import numpy as np
from functools import lru_cache

# Chance that config will get picked, even with lower new score
RANDOM_CHANCE_THRESHOLD = 0.4

# Threshold to continue trying new configurations
#  Below this, stop (reached convergence)
SCORE_DIFF_THRESHOLD = 5
NUM_ITERATIONS = 1000

# Weights for feature levels
PAIR_WEIGHT = 0.25
SUB_WEIGHT = 0.25
INC_WEIGHT = 0.25
SUPER_WEIGHT = 0.25

# When to update weights
TIME_TO_CHANGE = True

# Create features to evaluate
pair_features = []
sub_features = []
inc_features = []
sup_features = []


@lru_cache(maxsize=None)
def equal_country_code(group, feature):
    group = set(group)
    first = group.pop()
    on = all(first.country_code == item.country_code for item in group)

    return on, on * feature.weight


@lru_cache(maxsize=None)
def equal_src_actor(group, feature):
    group = set(group)
    first = group.pop()
    on = all(first.src_actor == item.src_actor for item in group)

    return on, on * feature.weight


@lru_cache(maxsize=None)
def equal_src_agent(group, feature):
    group = set(group)
    first = group.pop()
    on = all(first.src_agent == item.src_agent for item in group)

    return on, on * feature.weight


@lru_cache(maxsize=None)
def equal_tgt_actor(group, feature):
    group = set(group)
    first = group.pop()
    on = all(first.tgt_actor == item.tgt_actor for item in group)

    return on, on * feature.weight


@lru_cache(maxsize=None)
def equal_tgt_agent(group, feature):
    group = set(group)
    first = group.pop()
    on = all(first.tgt_agent == item.tgt_agent for item in group)

    return on, on * feature.weight


@lru_cache(maxsize=None)
def equal_geoname(group, feature):
    group = set(group)
    first = group.pop()
    on = all(first.geoname == item.geoname for item in group)

    return on, on * feature.weight


class Feature:
    """ Feature, with assoc function to call, weight, and boolean 
        to show if it was last 'on' (evaluated to True) """

    def __init__(self, feature_id, assoc_function):
        self.feature_id = feature_id
        self.assoc_function = assoc_function
        self.weight = 1
        self.on = False


class Sentence:
    """
    One sentence, with certain attributes attached.
    """

    def __init__(self, id, code, src_actor, src_agent, tgt_actor, tgt_agent, country_code, geoname):
        self.id = id
        self.sub_id = None

        self.code = code
        self.src_actor = src_actor
        self.src_agent = src_agent
        self.tgt_actor = tgt_actor
        self.tgt_agent = tgt_agent
        self.country_code = country_code
        self.geoname = geoname

    def __str__(self):
        return self.id


class SubIncident:
    """
    Groups together similar sentences to move them 
    together when reconfiguring Incidents.
    """

    def __init__(self, incident_id):
        self.id = id(self)
        self.incident_id = incident_id
        self.sentences = dict()
        self.features_on = np.zeros((len(sub_features),), dtype=int)

    def add_sentence(self, sentence):
        self.sentences[sentence.id] = sentence
        sentence.sub_id = self.id

    def remove_sentence(self, sent_id):

        del(self.sentences[sent_id])

    def get_sentences(self):
        return self.sentences.values()

    def calculate_score(self):
        # Total score for this sub-incident
        total = 0

        # Evaluate all relevant features for this sub incident
        for i, feature in enumerate(sub_features):
            on, score = feature.assoc_function(self.get_sentences(), feature)

            # Add to sum total score
            total += score * SUB_WEIGHT
            # Was this feature on this time?
            self.features_on[i] = on

        return total

    def __str__(self):
        return 'sub incident {}: '.format(self.id) + ' '.join(self.sentences.keys())


class Incident:
    """
    A collection of sub-incidents believed to 
    describe the same real-life event.
    """

    def __init__(self):
        self.id = id(self)
        self.super_id = None
        self.sub_incidents = dict()
        self.features_on = np.zeros((len(inc_features),), dtype=int)

    def add_sub_incident(self, sub_incident):
        self.sub_incidents[sub_incident.id] = sub_incident
        sub_incident.incident_id = self.id

    def remove_sub_incident(self, sub_id):
        self.sub_incidents[sub_id].incident_id = None
        del(self.sub_incidents[sub_id])

    def get_sentences(self):
        return list(sum((sub.get_sentences() for sub in self.sub_incidents), []))

    def calculate_score(self):
        # Total score for this sub-incident
        total = 0

        # Evaluate all relevant features for this sub incident
        for i, feature in enumerate(inc_features):
            on, score = feature.assoc_function(self.get_sentences(), feature)

            # Add to sum total score
            total += score * INC_WEIGHT
            # Was this feature on this time?
            self.features_on[i] = on

        return total

    def __str__(self):
        return 'incident {}: \n'.format(self.id) + '\n'.join([str(sub) for sub in self.sub_incidents.values()])


class SuperIncident:
    """
    Groups together similar Incident objects for purpose 
    of clustering on the same machines for computations.
    """

    def __init__(self):
        self.id = id(self)
        self.incidents = dict()
        self.features_on = np.zeros((len(sup_features),), dtype=int)

    def add_incident(self, incident):
        self.incidents[incident.id] = incident
        incident.super_id = self.id

    def remove_incident(self, inc_id):
        self.incidents[inc_id].super_id = None
        del(self.incidents[inc_id])

    def get_sentences(self):
        return list(sum((inc.get_sentences() for inc in self.incidents), []))

    def calculate_score(self):
        # Total score for this sub-incident
        total = 0

        # Evaluate all relevant features for this sub incident
        for i, feature in enumerate(sup_features):
            on, score = feature.assoc_function(self.get_sentences(), feature)

            # Add to sum total score
            total += score * SUPER_WEIGHT
            # Was this feature on this time?
            self.features_on[i] = on

        return total


class Configuration:
    """
    One configuration of super-incidents.
    """

    def __init__(self):
        self.super_incidents = dict()
        self.incidents = dict()
        self.sub_incidents = dict()
        self.sentences = set()

    def calculate_score(self):

        total = 0

        for pair in combinations(self.sentences.values(), 2):
            for feature in pair_features:

                on, result = feature.assoc_function(frozenset(pair))
                total += result * PAIR_WEIGHT

        for sup in self.super_incidents:
            total += sup.calculate_score()

        for inc in self.incidents:
            total += inc.calculate_score()

        for sub in self.sub_incidents:
            total += sub.calculate_score()

        return total


def gen_config_update(current_config):

    # Choose sub incident to move and where to move it
    sub_id = choice(current_config.sub_incidents.keys())
    old_inc_id = current_config.sub_incidents[sub_id].inc_id
    new_inc_id = choice([key for key in current_config.incidents.keys() if key != old_inc_id])

    # Create new configuration
    new_config = deepcopy(current_config)

    # Clean up references
    new_config.sub_incidents[sub_id].inc_id = new_inc_id
    new_config.incidents[old_inc_id].remove_sub_incident(sub_id)
    new_config.incidents[new_inc_id].add_sub_incident(new_config.sub_incidents[sub_id])

    return new_config


def gen_sentences(filename):
    sentences = []

    # Open up json and iterate through
    doc = json.load(open(filename, 'r'))

    for elt in doc:
        # Get relevant attributes
        id = elt['id'] if 'id' in elt else None
        code = elt['code'] if 'code' in elt else None
        src_actor = elt['src_actor'] if 'src_actor' in elt else None
        src_agent = elt['src_agent'] if 'src_agent' in elt else None
        tgt_actor = elt['tgt_actor'] if 'tgt_actor' in elt else None
        tgt_agent = elt['tgt_agent'] if 'tgt_agent' in elt else None
        country_code = elt['country_code'] if 'country_code' in elt else None
        geoname = elt['geoname'] if 'geoname' in elt else None

        # Create a sentence
        sentence = Sentence(id, code, src_actor, src_agent, tgt_actor, tgt_agent, country_code, geoname)
        sentences.append(sentence)

    return sentences


def gen_random_config(sentences, num_incs, num_subs):

    config = Configuration()
    incs = dict()
    subs = dict()

    for i in range(num_incs):
        inc = Incident()
        incs[inc.id] = inc

    config.incidents = incs

    for i in range(num_subs):
        inc_choice = choice(list(incs.keys()))
        sub = SubIncident(inc_choice)
        config.incidents[inc_choice].add_sub_incident(sub.id)
        subs[sub.id] = sub

    config.sub_incidents = subs

    for sent in sentences:
        sub_choice = choice(list(subs.keys()))
        config.sub_incidents[sub_choice].add_sentence(sent)

    return Configuration()


def calculate_feat_updates(updates, prev_config, new_config):

    new_updates = []

    for sub in new_config.sub_incidents:
        # Gather up of/on stats for features
        new_feats = new_config.sub_incidents[sub]

        # This is a new sub incident
        if sub not in prev_config.sub_incidents:
            prev_feats = np.zeros((len(sub_features),), dtype=int)
        else:  # This was already here
            prev_feats = prev_config.sub_incidents[sub]

        # Subtract the two to see what we turned on/off
        diff = np.subtract(new_feats, prev_feats)

        # Add to sum total updates
        new_updates.append(np.add(updates[0], diff))

    for inc in new_config.incidents:
        # Gather up of/on stats for features
        new_feats = new_config.incidents[inc]

        # This is a new sub incident
        if inc not in prev_config.incidents:
            prev_feats = np.zeros((len(inc_features),), dtype=int)
        else:  # This was already here
            prev_feats = prev_config.inc_incidents[inc]

        # Subtract the two to see what we turned on/off
        diff = np.subtract(new_feats, prev_feats)

        # Add to sum total updates
        new_updates.append(np.add(updates[1], diff))

    for sup in new_config.super_incidents:
        # Gather up of/on stats for features
        new_feats = new_config.super_incidents[sup]

        # This is a new sub incident
        if sup not in prev_config.super_incidents:
            prev_feats = np.zeros((len(sup_features),), dtype=int)
        else:  # This was already here
            prev_feats = prev_config.super_incidents[sup]

        # Subtract the two to see what we turned on/off
        diff = np.subtract(new_feats, prev_feats)

        # Add to sum total updates
        new_updates.append(np.add(updates[2], diff))

    return new_updates


def experiment():

    # Pending updates to feature weights
    pair_feat_updates = np.zeros((len(pair_features),), dtype=int)
    sub_feat_updates = np.zeros((len(sub_features),), dtype=int)
    inc_feat_updates = np.zeros((len(inc_features),), dtype=int)
    sup_feat_updates = np.zeros((len(sup_features),), dtype=int)

    updates = [sub_feat_updates, inc_feat_updates, sup_feat_updates]

    # Scores to plot and show convergence
    scores = []
    start_time = time()
    times = [start_time]

    # Generate objects to represent data
    filename = 'datawithdoc.txt'
    sentences = gen_sentences(filename)

    # Create a random starting configuration
    current_config = gen_random_config(sentences, 20, 60)
    current_score = current_config.calculate_score()
    # score_diff = math.inf

    # Allow for eventual convergence
    # while score_diff > SCORE_DIFF_THRESHOLD:

    for iteration in range(NUM_ITERATIONS):

        # Create a new trial configuration
        new_config = gen_config_update(current_config)
        new_score = new_config.calculate_score()
        score_diff = new_score - current_score

        # If score difference is positive or this is chosen to randomly change
        if score_diff > 0 or random() > RANDOM_CHANCE_THRESHOLD:

            # Record changes to feature weights
            updates = calculate_feat_updates(updates, current_config, new_config)

            # Update configuration
            del current_config
            current_config = new_config
            current_score = new_score

            # Record to plot later
            scores.append(current_score)
            times.append(time() - start_time)

            # Is it time to update feature weights?
            if TIME_TO_CHANGE:

                # Update all weights as necessary
                for feat_list in (pair_features, sub_features, inc_features, sup_features):

                    for feat in feat_list:
                        feat_list[feat][0].weight += feat_list[feat][1]
                        feat_list[1] = 0  # reset running weight update sum

    # plot results
    plt.plot(times, scores)
    plt.xlabel('Time')
    plt.ylabel('Configuration Score')
    plt.show()
    plt.savefig('scores.png', bbox_inches='tight')


if __name__ == '__main__':
    experiment()