package edu.oklahoma.oudalab.coincidents;

import com.amazon.lattice.emr.AbstractSparkJob;
import org.apache.spark.api.java.JavaRDD;
import org.apache.spark.api.java.JavaSparkContext;
import org.apache.spark.api.java.function.Function;
import org.apache.spark.sql.Dataset;
import org.apache.spark.sql.Row;
import org.apache.spark.sql.SaveMode;
import org.apache.spark.sql.SparkSession;
import org.apache.spark.sql.expressions.Window;
import org.kohsuke.args4j.Option;

import lombok.NonNull;
import lombok.extern.slf4j.Slf4j;

import java.io.Serializable;
import java.util.Map;

import static org.apache.spark.sql.functions.col;
import static org.apache.spark.sql.functions.row_number;

/**
 * This Spark job loads the event dataset, runs blocking, merging,
 * and linkage on the event dataset to compute the co-incidents.
 */

@Slf4j
public class ComputeCoIncidentsSparkJob extends AbstractSparkJob implements Serializable {

    private static final long serialVersionUID = 1L;

    @Option(name = "--eventS3Path", required = true, usage = "The s3 path of the event dataset")
    private String eventS3Path;

    private SparkSession sparkSession;
    private CoIncidentsLinker coIncidentsLinker;

    @Override
    protected void run(@NonNull JavaSparkContext javaSparkContext) {

        log.info("Enters the Spark job ComputeCoIncidentsSparkJob");

        sparkSession = SparkSession.builder().getOrCreate();
        coIncidentsLinker = new CoIncidentsLinker();

        Dataset<Row> eventDataset = sparkSession.read().json(eventS3Path);
        eventDataset = eventDataset.withColumn("groupId", row_number().over(Window.orderBy("date8"))); 
        log.info("The size of eventDataset is: {}", eventDataset.count());



        /*
        String[] dimensions = {
            "target",
            "longitude",
            //"stategeonameid",
            "tgt_actor",
            "src_actor",
            "date8",
            "code",
            "countrycode",
            "root_code",
            "geoname",
            //"countrygeonameid",
            "tgt_agent",
            //"month",
            "src_agent",
            //"mongo_id",
            "tgt_other_agent",
            //"year",
            "statecode",
            "latitude",
            "source", //media source
            //"day",
            "target",
            "embed"};
        */

        String[] dimensions = {
            "date8"
        };

        for (String dimension : dimensions) {
            eventDataset = coIncidentsLinker.runLinkage(dimension, eventDataset);
            log.info("Size of eventDataset is {} after dealing dimension {}", eventDataset.count(), dimension);
        }

        String s3LinkedEventsPath = "s3://shupingj-gamma/linked-events";
        eventDataset.write().mode(SaveMode.Overwrite).parquet(s3LinkedEventsPath);
    }
}
