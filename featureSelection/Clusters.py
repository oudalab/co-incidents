"""
This file defines all cluster types, including a base Cluster class and its 
subclasses Pair, SubIncident, Incident, and SuperIncident. It also defines a 
Configuration class to keep track of all of these for a grouping of data points. 

@author Elena Montes
"""


import numpy as np

from abc import ABCMeta, abstractmethod
from itertools import combinations


'''-------------------------------------------------------------------------------------------------------'''
''' The abstract base class for a Cluster (pair, sub incident, incident, super incident) within the       '''
''' Configuration. Lays out concrete method for calculating score of any cluster based on its associated  '''
''' features. Implements abstract method for getting list of sentences (including from sub clusters).     '''
'''-------------------------------------------------------------------------------------------------------'''


class Cluster(metaclass=ABCMeta):
    """
    Abstract base class for any kind of cluster. Implements score calculation for 
    all features involving any subclass features.
    """

    def __init__(self, num_features):
        """
        Assigns an id for this cluster, along with a list of the features 
        evaluated to True for this cluster.
        :param num_features: number of features to be evaluated
        """
        self.id = id(self)
        self.features_on = np.zeros((num_features,), dtype=int)

    def calculate_score(self, features):
        """
        Calculate total score for this cluster based on given features. 
        Records which features turned on.
        :param features: features to be evaluated
        :return: total score as a float
        """
        # Total score for this sub-incident
        total = 0.0

        # Evaluate all relevant features for this sub incident
        for i, feature in enumerate(features):

            on = feature.assoc_function(frozenset(self.get_sentences()), feature)
            total += on * feature.weight

            # Record result of feature
            self.features_on[i] = on

        return total

    @abstractmethod
    def get_sentences(self):
        """
        Abstract method to return all sentences contained within this cluster. 
        Implemented by subclasses.
        :return: list of sentences
        """
        raise NotImplementedError


'''-------------------------------------------------------------------------------------------------------'''
''' The implementations for the different hierarchical levels of Clusters within a Configuration.         '''
'''-------------------------------------------------------------------------------------------------------'''


class Pair(Cluster):
    """
    One pair of two sentences (must be clustered together in another, larger cluster). 
    """

    def __init__(self, num_features, sentences):
        """ 
        Initialize this pair with list of two sentences.
        """
        super(Pair, self).__init__(num_features)
        self.sentences = sentences

    def __str__(self):
        """ 
        :return: string representation of this cluster. 
        """
        return 'pair {}: '.format(self.id) + ' '.join(self.sentences)

    def get_sentences(self):
        """ 
        :return: list of all sentences within this cluster. 
        """
        return self.sentences.values()


class SubIncident(Cluster):
    """
    Groups together similar sentences to move them together when reconfiguring Incidents.
    """
    
    def __init__(self, num_features, inc_id):
        """ 
        Initialize this sub incident with id of its parent incident. 
        """
        super(SubIncident, self).__init__(num_features)
        self.inc_id = inc_id
        self.sentences = dict()

    def __str__(self):
        """ 
        :return: string representation of this cluster. 
        """
        return 'sub incident {}: '.format(self.id) + ' '.join(self.sentences.keys())

    def get_sentences(self):
        """ 
        :return: list of all sentences within this cluster. 
        """
        return self.sentences.values()

    def add_sentence(self, sentence):
        """ 
        Add provided sentence to this sub incident. 
        :param sentence: sentence object to be added 
        """
        self.sentences[sentence.id] = sentence
        sentence.sub_id = self.id

    def remove_sentence(self, sent_id):
        """ 
        Remove and return sentence with given id. 
        :param sent_id: id of the sentence to be removed 
        :return: sentence object 
        """
        sentence = self.sentences[sent_id]
        del self.sentences[sent_id]
        return sentence


class Incident(Cluster):
    """
    A collection of sub-incidents believed to describe the same real-life event.
    """

    def __init__(self, num_features, super_id):
        """ 
        Initialize this incident with id of its parent super incident. 
        """
        super(Incident, self).__init__(num_features)
        self.super_id = super_id
        self.sub_incidents = dict()

    def __str__(self):
        """ 
        :return: string representation of this cluster. 
        """
        return 'incident {}: \n'.format(self.id) + '\n'.join([str(sub) for sub in self.sub_incidents.values()])

    def get_sentences(self):
        """ 
        :return: list of all sentences within this cluster. 
        """
        return list(sum((list(sub.get_sentences()) for sub in self.sub_incidents.values()), []))

    def add_sub_incident(self, sub_incident):
        """ 
        Add provided sub incident to this incident. 
        :param sub_incident: sub incident object to be added 
        """
        self.sub_incidents[sub_incident.id] = sub_incident
        sub_incident.inc_id = self.id

    def remove_sub_incident(self, sub_id):
        """ 
        Remove and return sub incident with given id. 
        :param sub_id: id of the sub incident to be removed
        :return: sub incident object 
        """
        sub_incident = self.sub_incidents[sub_id]
        del self.sub_incidents[sub_id]
        return sub_incident


class SuperIncident(Cluster):
    """
    Groups together similar Incident objects for purpose of clustering on the same machines.
    """

    def __init__(self, num_features):
        """ 
        Initialize this super incident. 
        """
        super(SuperIncident, self).__init__(num_features)
        self.incidents = dict()

    def __str__(self):
        """ 
        :return: string representation of this cluster. 
        """
        return 'super incident {}: \n'.format(self.id) + '\n'.join([str(sub) for sub in self.incidents.values()])

    def get_sentences(self):
        """ 
        :return: list of all sentences within this cluster. 
        """
        return list(sum(list(inc.get_sentences() for inc in self.incidents.values()), []))

    def add_incident(self, incident):
        """ 
        Add provided incident to this super incident. 
        :param incident: incident object to be added 
        """
        self.incidents[incident.id] = incident
        incident.inc_id = self.id

    def remove_incident(self, inc_id):
        """ 
        Remove specified incident from this cluster. 
        :param inc_id: id for incident to be removed
        :return: the incident object
        """
        incident = self.incidents[inc_id]
        del self.incidents[inc_id]
        return incident


'''-------------------------------------------------------------------------------------------------------'''
''' Below, the implementation for the general Configuration of sentences into clusters described above.   '''
'''-------------------------------------------------------------------------------------------------------'''


class Configuration:
    """ 
    One configuration of super-incidents. 
    """

    def __init__(self, supers, incs, subs, sup_feats, inc_feats, sub_feats, pair_feats):
        """ 
        Initialize this configuration with specified attributes. 
        """
        self.id = id(self)

        self.super_incidents = supers
        self.incidents = incs
        self.sub_incidents = subs
        self.pairs = dict()

        for sub in self.sub_incidents:
            for pair in list(combinations(sub.get_sentences(), 2)):
                pair_obj = Pair(len(pair_feats), list(pair))
                self.pairs[pair_obj.id] = pair_obj

        self.sup_feats = sup_feats
        self.inc_feats = inc_feats
        self.sub_feats = sub_feats
        self.pair_feats = pair_feats

    def __str__(self):
        """ 
        :return: string representation of this configuration. 
        """
        return 'configuration {}: \n'.format(self.id) + '\n\n'.join([str(sup) for sup in self.super_incidents.values()])

    def get_sentences(self):
        """ 
        :return: list of all sentences within this configuration.
        """
        return list(sum(list(sup.get_sentences() for sup in self.super_incidents.values()), []))

    def calculate_score(self):
        """
        Calculate score based on all super incidents, incidents, sub incidents, and 
        grouped pairs of sentences in this configuration.
        :return: a score as a float value
        """
        total = 0.0

        for sup in self.super_incidents.values():
            total += sup.calculate_score(self.sup_feats)

        for inc in self.incidents.values():
            total += inc.calculate_score(self.inc_feats)

        for sub in self.sub_incidents.values():
            total += sub.calculate_score(self.sub_feats)

        for pair in self.pairs.values():
            total += pair.calculate_score(self.pair_feats)

        return total
