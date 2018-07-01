# Computational Political Science
Using a PMG model to constructing the cluster for political event data.

## co-incidence resolution (clustering)
Using hierarchical MCMC with Metropolis Hastings algorithm to cluster news corpus into appropriate cluster.
tfidf embedding of the article and also event property that extrated from Petrach2 will be used as the learning feature.
We implement SampleRank to learn the weight for each feature in the graph.

## event tracking resolution (search the causality track)
give two events, how to efficiently output the causality track of the event A and Z
say we should output something like A->B->C->D...->Z

## pattern aware search MCMC sampling
so given a pattern what is the qucik way to search out a set of events that have this pattern in a large dataset:
for example:
kill-> protest-> kill.
