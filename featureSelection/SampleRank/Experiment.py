"""
This file contains the actual experiment (with command line + default arguments),
alongside a couple of supporting functions. 

IMPORTANT NOTE: Be sure to create sub-directories 'clusters', 'run_logs', and 
'graphs' if they do not already exist, so that outputs save out properly.

@author: Elena Montes
"""

import click
import json
import logging
import matplotlib.pyplot as plt
import numpy as np

from random import random, sample
from time import time, localtime, asctime

from Definitions import Sentence, Incident, Configuration
from Features import get_features


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


@click.command()
@click.option('--filename', default='../datawithdoc.txt', help='Name of file with data')
@click.option('--is_clustered', default=True, help='Start all sentences together or separate')
@click.option('--num_iterations', default=10000, help='Number of iterations to run')
@click.option('--prob_isolation', default=0.4,
              help='prob [0.0, 1.0] that an incident will be moved to its own new incident')
@click.option('--num_samples', default=1000, help='Number of samples to run simulation')
def experiment(filename, is_clustered, num_iterations, prob_isolation, num_samples):

    # Generate a label to tag all outputs
    run_num = asctime(localtime(time())).replace(' ', '_')
    logging.basicConfig(filename='run_logs/run_{}.log'.format(run_num), level=logging.INFO)

    # Get features and initialize running update sums
    pair_feats, incident_feats = get_features()
    feature_updates = np.zeros((len(incident_feats),), dtype=int)

    # Generate objects to represent data
    sentences, docs = gen_sentences(filename)
    logging.info('COMPLETE: {} sentences generated.\n'.format(len(sentences)))

    # Label some docs to track - label half total samples
    labeled_docs = dict()
    for actor in {'USA', 'RUS', 'AUS', 'JPN', 'MEX', 'KEN', 'BRA', 'CAN', 'ZAF', 'CHN'}:
        # 10% for each label type
        actor_docs = [s for s in sentences if s.src_actor == actor][:num_samples / 20]
        doc_updates = {se.id_num: se for se in actor_docs}
        labeled_docs.update(doc_updates)

    samples = [s for s in labeled_docs.values()]

    # Fill in other samples
    samples += sample([s for s in sentences if s not in samples], num_samples / 2)

    logging.info('{} sentences chosen for run sample.'.format(len(samples)))

    # Create a random starting configuration
    current_config = gen_starting_config(is_clustered, samples, len(incident_feats), len(pair_feats))
    logging.info('COMPLETE: random configuration created.\n')

    # Score this configuration
    current_score = current_config.calculate_score(incident_feats, pair_feats)
    logging.info('COMPLETE: calculated initial score of {}.\n'.format(current_score))

    # Scores to plot and show convergence
    scores = []
    iterations = []

    # Keep track of percentage of accepted proposals
    times_accepted = 0

    # Run for specified number of iterations
    for iter_num in range(num_iterations):

        logging.info('Beginning iteration {}.\n'.format(iter_num))

        # Create a new trial configuration
        new_config, log = current_config.gen_config_update(prob_isolation, len(incident_feats), len(pair_feats))
        new_score = new_config.calculate_score(incident_feats, pair_feats)
        score_diff = new_score - current_score

        # Metropolis Hastings probability of acceptance
        # temperature = 0.01
        mh_acceptance = (new_score / current_score)  # ** (1/temperature)

        # Acceptance test: score diff is positive or is chosen to randomly change
        if random() <= min(1.0, mh_acceptance):

            times_accepted += 1

            logging.info('New configuration accepted. Score update: {}'.format(score_diff))
            logging.info(log)

            # Record changes to feature weights
            feature_updates = current_config.calculate_feat_updates(feature_updates, new_config, len(incident_feats))

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
                for i, feat in enumerate(incident_feats):
                    feat.weight += feature_updates[i] / total_weight if total_weight != 0 else 0
                    feature_updates[i] = 0  # reset running weight update sum

    for i, feat in enumerate(incident_feats):
        logging.info('Feature: {} | Accumulated Weight: {}'.format(feat.feature_id, feature_updates[i]))

    # Log relevant info about overall purity and acceptance rate
    logging.info('Purity: {}'.format(current_config.calculate_accuracy(labeled_docs)))
    logging.info('Percentage of Acceptance: {}'.format(times_accepted / num_iterations))

    # Write out cluster data to a file (for closer inspection)
    cluster_file = open('clusters/{}.txt'.format(run_num), 'w')
    cluster_file.write(str(current_config))
    cluster_file.close()

    # Plot results and save to file
    plt.plot(iterations, scores)
    plt.xlabel('Iteration')
    plt.ylabel('Configuration Score')
    plt.savefig('graphs/run_{}_scores.png'.format(run_num), bbox_inches='tight')
    plt.show()


if __name__ == '__main__':
    experiment()
