package edu.oklahoma.oudalab.coincidents;

import org.apache.spark.sql.Dataset;
import org.apache.spark.sql.Row;

import java.io.Serializable;
import java.util.Map;
import java.util.HashMap;

import static org.apache.spark.sql.functions.col;
import static org.apache.spark.sql.functions.desc;


public class CoIncidentsLinker implements Serializable {

    public Dataset<Row> runLinkage(String dimension, Dataset<Row> events) {

        Dataset<Row> partitionedEvents = events.repartition(200, col(dimension)); 
        Dataset<Row> orderedEvents = partitionedEvents.orderBy(desc(dimension));

        //todo: partition/groupby + flatmap, etc to run linkage.
        //result will contain oldGroup -> new group || large group -> small group
        //orderEvents.flatMap();

        return orderedEvents;
    }
}
