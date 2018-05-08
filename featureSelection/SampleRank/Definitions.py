"""
This file contains all definitions for mocking a simplified version of the MCMC clustering 
algorithm (just a Configuration with Incident clusters of Sentences). An Incident can score 
itself given some features, as can a Configuration. A Configuration can calculate its final 
cluster purity based on some labeled docs and can generate an update for itself by choosing 
a Sentence to move at random.

@author Elena Montes
"""

import numpy as np

from copy import deepcopy
from random import choice, random


class Sentence:

    def __init__(self, id_num, code, src_actor, src_agent, src_other_agent, tgt_actor,
                 tgt_agent, tgt_other_agent, country_code, geoname, latitude, longitude):
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
        return 'Code: {} | Src_actor: {} | Src_agent: {} | Tgt_actor: {} | Tgt_agent: {} | Country: {} | Geoname: {}'\
            .format(self.code, self.src_actor, self.src_agent, self.tgt_actor, self.tgt_agent, self.country_code, self.geoname)


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

    def __str__(self):
        representation = 'Incident {}\n'.format(self.id)
        for s in self.get_sentences():
            representation += str(s) + '\n'

        return representation


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
        total = len(labeled_samples)
        for incident in self.incidents.values():
            # Get all labeled points in this incident cluster
            data = [s for s in incident.sentences.keys() if s in labeled_samples]
            if data:
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

        # Return purity for all clusters
        return num_correct/total

    def gen_config_update(self, prob_isolation, num_inc_features, num_pair_features):
        """
        Generate a potential new configuration. Chance of isolation tells us if the 
        randomly chosen sentence will be put into a brand new incident by itself or
        simply moved between incidents.
        :param prob_isolation: chance of moving or isolating
        :param num_inc_features: number of incident features
        :param num_pair_features: number of pairwise features
        :return: new configuration object
        """
        # Choose sentence to move
        sentence = choice(self.get_sentences())
        old_inc_id = sentence.inc_id

        # Create new configuration
        new_config = deepcopy(self)

        # Record events in case this config is accepted
        log = ''

        if random() <= prob_isolation or len(new_config.incidents) == 1:
            log += 'Isolated sentence {}\n'.format(sentence.id_num)

            # We will create a new incident with just this sentence
            new_incident = Incident(num_inc_features, num_pair_features)
            new_config.incidents[new_incident.id] = new_incident
        else:
            # Choose where to move this sentence
            new_incident = choice(list([i for i in new_config.incidents.values() if i.id != old_inc_id]))
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

        return new_config, log
