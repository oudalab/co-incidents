package edu.oklahoma.oudalab.coincidents;

import org.apache.spark.sql.Dataset;
import org.apache.spark.sql.Row;

import java.io.Serializable;
import java.util.Map;
import java.util.HashMap;


public class CoIncidentsLinker implements Serializable {

    public Map<Integer, Integer> runLinkage(String dimension, Dataset events) {

        Map<Integer, Integer> linkedGroups = new HashMap<Integer, Integer>();

        return linkedGroups;
    }

}
