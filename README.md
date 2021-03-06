# Computational Political Science
Using PMG to model the relation between events. 


## co-incidence resolution (clustering)
* for linking part:
Using hierarchical MCMC with Metropolis Hastings algorithm to cluster news corpus into appropriate cluster.(basically we are solving a MAP problem, and using MCMC as the sampling method for inferecen) http://sameersingh.org/files/papers/largescale-acl11.pdf
tfidf embedding of the article and also event properties that are extrated from Petrach2 will be used as learning features.
(Sample rank to determine the attribute weight is not implemented here)

* for blocking part:
we implemented by using spark by using the following logic. (at the same time we are aware of the sub-cluster and super-cluster logic)
  * for subcluster we mean that when we try to merge two "events" together, we will merge the query event to the target    "incidence" not the the target event itself
  * for supercluster we mean by choosing clever partion, the similar incidence will be moved to the same node, in this way, they have better chance to be linked.
  * We model the blocking idea as follows: so we are going to block by the event attribute: say time, actor, target, geolocation, eventcode.
  we constrcut an dataframe in spark, and each row represent an event (E) , for each block iteration, we can generally model the input for each layer as a list of incidence (a set of events that has been merged together denoted as I), and then after merge on the current blocking layer, we output it as a list of incidence.
  Following is the graph to illustrate this.
  
  ![alt text](https://github.com/oudalab/co-incidents/blob/master/experiments/blocking.jpg)
  on each iteration we will do 4 steps:
    1. repartion by the blocking key
    1. within the repartion do the linkage
    1. merge the Incidence coming from different partition based on the previous incidence key
    1. update the attributes value on the incidence (say Incidence should have a counrty list, when new event added into the incidence, we need to update the the country list for this Incidence. 
  
Example:
  say after blokcing by year and did the first round linkage, we get E1 and E5 belongs to I1,
  E2 and E4 belongs to I2, E3 and E6 belongs to I3. then we block by Actor , so E1 and E2 will be on the same partion, E3 and E4 will be on the same partition and E5, E6 will be on the same partiton.  and within each partion we do the linkage, then merged the list of output incidence from each partion and update the merged incidence attribues. 
  two good things of blocking in this way:
   * when doing partion, we only need to care about one dimension say only Actor values for the second round partition
   * by merging the incidence coming from the output of the linkage, we don't lose the information from the previous blocking iteration result
  ![alt text](https://github.com/oudalab/co-incidents/blob/master/experiments/process.jpg)
  
## event evloving (event diffusion)
which kind of event is evolving along time, since we our event data is temporal.
## event tracking resolution (search the causality track)
give two events, how to efficiently output the causality track of the event A and Z
say we should output something like A->B->C->D...->Z

## pattern aware search MCMC sampling
so given a pattern what is the qucik way to search out a set of events that have this pattern in a large dataset:
for example:
kill-> protest-> kill.
/home/yan/coincidenceData

## event visualization and user interaction with the visulization.

# steps to get the tfidf data from the sampleEventData.json
 >run tfidf.py first which will generate the bow_features model and bow_vectors model
 >run generateEmbeddingData.py will add the sentence vector to the output file, the output file is the one we are going to use
  for training.

