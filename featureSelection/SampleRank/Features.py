"""
This file contains definitions of features based on the arguments present in a Sentence.
Packages feature check functions within Feature objects that can track if that feature
was last True or False; contains method to get all defined features.
Uses lru_cache to reduce computational load on repeated identical calls. 

@author Elena Montes
"""

from functools import lru_cache


@lru_cache(maxsize=None)
def eq_country_code(group):
    if len(group) > 0:
        group = set(group)
        if all([s.country_code == '' for s in group]):
            return False
        first = group.pop()
        on = all(first.country_code == item.country_code for item in group)
        return on

    return True


@lru_cache(maxsize=None)
def eq_src_actor(group):
    if len(group) > 0:
        group = set(group)
        if all([s.src_actor == '' for s in group]):
            return False
        first = group.pop()
        on = all(first.src_actor == item.src_actor for item in group)
        return on

    return True


@lru_cache(maxsize=None)
def eq_src_agent(group):
    if len(group) > 0:
        group = set(group)
        if all([s.src_agent == '' for s in group]):
            return False
        first = group.pop()
        on = all(first.src_agent == item.src_agent for item in group)
        return on

    return True


@lru_cache(maxsize=None)
def eq_src_other_agent(group):
    if len(group) > 0:
        group = set(group)
        if all([s.src_other_agent == '' for s in group]):
            return False
        first = group.pop()
        on = all(first.src_other_agent == item.src_other_agent for item in group)
        return on

    return True


@lru_cache(maxsize=None)
def eq_tgt_actor(group):
    if len(group) > 0:
        group = set(group)
        if all([s.tgt_actor == '' for s in group]):
            return False
        first = group.pop()
        on = all(first.tgt_actor == item.tgt_actor for item in group)
        return on

    return True


@lru_cache(maxsize=None)
def eq_tgt_agent(group):
    if len(group) > 0:
        group = set(group)
        if all([s.tgt_agent == '' for s in group]):
            return False
        first = group.pop()
        on = all(first.tgt_agent == item.tgt_agent for item in group)
        return on

    return True


@lru_cache(maxsize=None)
def eq_tgt_other_agent(group):
    if len(group) > 0:
        group = set(group)
        if all([s.tgt_other_agent == '' for s in group]):
            return False
        first = group.pop()
        on = all(first.tgt_other_agent == item.tgt_other_agent for item in group)
        return on

    return True


@lru_cache(maxsize=None)
def eq_geoname(group):
    if len(group) > 0:
        group = set(group)
        if all([s.geoname == '' for s in group]):
            return False
        first = group.pop()
        on = all(first.geoname == item.geoname for item in group)
        return on

    return False


@lru_cache(maxsize=None)
def eq_code(group):
    if len(group) > 0:
        group = set(group)
        if all([s.code == '' for s in group]):
            return False
        first = group.pop()
        on = all(first.code == item.code for item in group)
        return on

    return False


class Feature:
    """ Feature, with assoc function to call, weight, and boolean 
        to show if it was last 'on' (evaluated to True) """

    def __init__(self, feature_id, assoc_function):
        self.feature_id = feature_id
        self.assoc_function = assoc_function
        self.weight = 1
        self.on = False


def get_features():
    """
    Return nicely packaged lists of Feature objects for pairwise and incident-wide 
    scoring purposes. (Keeps track of weight update over time)
    :return: list of pair features and list of incident features
    """
    pair_eql_country = Feature('ecc', eq_country_code)
    inc_eql_country = Feature('ecc', eq_country_code)

    pair_eql_src_ac = Feature('esac', eq_src_actor)
    inc_eql_src_ac = Feature('esac', eq_src_actor)

    pair_eql_src_ag = Feature('esag', eq_src_agent)
    inc_eql_src_ag = Feature('esag', eq_src_agent)

    pair_eql_src_o_ag = Feature('esoag', eq_src_other_agent)
    inc_eql_src_o_ag = Feature('esoag', eq_src_other_agent)

    pair_eql_tgt_ac = Feature('etac', eq_tgt_actor)
    inc_eql_tgt_ac = Feature('etac', eq_tgt_actor)

    pair_eql_tgt_ag = Feature('etag', eq_tgt_agent)
    inc_eql_tgt_ag = Feature('etag', eq_tgt_agent)

    pair_eql_tgt_o_ag = Feature('etoag', eq_tgt_other_agent)
    inc_eql_tgt_o_ag = Feature('etoag', eq_tgt_other_agent)

    pair_eql_geo = Feature('eg', eq_geoname)
    inc_eql_geo = Feature('eg', eq_geoname)

    pair_eql_code = Feature('ec', eq_code)
    inc_eql_code = Feature('ec', eq_code)

    pair_feats = [pair_eql_src_ac, pair_eql_src_ag, pair_eql_src_o_ag, pair_eql_tgt_ac, pair_eql_tgt_ag,
                  pair_eql_tgt_o_ag, pair_eql_country, pair_eql_geo, pair_eql_code]
    incident_feats = [inc_eql_src_ac, inc_eql_src_ag, inc_eql_src_o_ag, inc_eql_tgt_ac, inc_eql_tgt_ag,
                      inc_eql_tgt_o_ag, inc_eql_country, inc_eql_geo, inc_eql_code]

    return pair_feats, incident_feats
