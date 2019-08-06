package edu.oklahoma.oudalab.coincidents;

import org.apache.spark.api.java.function.MapFunction;
import org.apache.spark.api.java.function.FlatMapGroupsFunction;
import org.apache.spark.sql.Dataset;
import org.apache.spark.sql.Encoders;
import org.apache.spark.sql.KeyValueGroupedDataset;
import org.apache.spark.sql.Row;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.HashMap;

import lombok.extern.slf4j.Slf4j;

import static org.apache.spark.sql.functions.col;
import static org.apache.spark.sql.functions.desc;


@Slf4j
public class CoIncidentsLinker implements Serializable {

    public Dataset<Row> runLinkage(String dimension, Dataset<Row> events) {

        log.info("Start to run linkage on the dimension: " + dimension);

        KeyValueGroupedDataset<String, Row> groupedEvents = events.groupByKey(new MapFunction<Row, String>() {
            @Override
            public String call(Row row) throws Exception {
                return row.getAs(dimension);
            }
        }, Encoders.bean(String.class));

        log.info("The size of groupedEvents is: " + groupedEvents.count());

        Dataset<Row> linkedEvents = groupedEvents.flatMapGroups(new FlatMapGroupsFunction<String, Row, Row>() {
            @Override
            public Iterator<Row> call(String dimension, Iterator<Row> rowIter) throws Exception {

                // todo: @Yan, please provide your implementation here
                List<Row> newRows = new ArrayList<>();
                while (rowIter.hasNext()) {
                    Row oldRow = rowIter.next();
                    newRows.add(oldRow);
                }
                return newRows.iterator();
            }
        }, Encoders.bean(Row.class));

        return linkedEvents;
    }
}
