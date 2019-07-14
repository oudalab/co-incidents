package edu.oklahoma.oudalab.coincidents;

import org.apache.spark.sql.Dataset;
import org.apache.spark.sql.Row;

import java.io.Serializable;
import java.util.Map;
import java.util.HashMap;

import static org.apache.spark.sql.functions.col;
import static org.apache.spark.sql.functions.desc;


public class CoIncidentsLinker implements Serializable {

    public Map<Integer, Integer> runLinkage(String dimension, Dataset<Row> events) {

        Map<Integer, Integer> linkedGroups = new HashMap<Integer, Integer>();
        Dataset<Row> partitionedEvents = events.repartition(col(dimension)); 
        Dataset<Row> orderedEvents = partitionedEvents.orderBy(desc(dimension));

        //orderEvents.flatMap();
        //orderEvents.foreachPartition();

        return linkedGroups;
    }

}
