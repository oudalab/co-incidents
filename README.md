# Computational Political Science
Using PMG to model the relation between events. 


## co-incidence resolution (clustering)
Using hierarchical MCMC with Metropolis Hastings algorithm to cluster news corpus into appropriate cluster.
tfidf embedding of the article and also event properties that are extrated from Petrach2 will be used as learning features.
We implement SampleRank to learn the weight for each feature in the graph.
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

