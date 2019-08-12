package edu.oklahoma.oudalab.coincidents;

import com.google.common.collect.ImmutableMap;
import java.util.*;
import java.util.concurrent.ThreadLocalRandom;
import org.apache.commons.lang.StringUtils;
import org.apache.logging.log4j.util.StringMap;
import org.apache.spark.api.java.function.MapFunction;
import org.apache.spark.api.java.function.FlatMapGroupsFunction;
import org.apache.spark.sql.*;

import java.io.Serializable;

import lombok.extern.slf4j.Slf4j;

import static org.apache.spark.sql.functions.col;
import static org.apache.spark.sql.functions.desc;


@Slf4j
public class CoIncidentsLinker implements Serializable {

    private static final double threshold = 12.0;
    private static final double similarity_threshold = 0.8;
    private static List<String> featureList = new ArrayList<>();
    /**
     * weight for each feature and these weight can be learned later.
     */
    private static final Map<String, Integer> weightMap = ImmutableMap.<String, Integer>builder()
            .put("target", 1)
            .put("src_actor", 1)
            .put("tgt_actor", 1)
            .put("code", 2)
            .put("countrycode", 1)
            .put("geoname", 1)
            .put("src_agent", 1)
            .put("tgt_agent", 1)
            .put("embed", 1)
            .build();

    public CoIncidentsLinker() {
        featureList.add("target");
        featureList.add("src_actor");
        featureList.add("tgt_actor");
        featureList.add("code");
        featureList.add("countrycode");
        featureList.add("geoname");
        featureList.add("src_agent");
        featureList.add("tgt_agent");
        featureList.add("embed");
    }

    /**
     * calculate cosine similarity of two embeddings
     *
     * @param embed1
     * @param embed2
     * @return
     */
    private static double cosineSimilarity_single(double[] embed1, double[] embed2) {
        double dotProduct = 0.0;
        double normA = 0.0;
        double normB = 0.0;
        for (int i = 0; i < embed1.length; i++) {
            dotProduct += embed1[i] * embed2[i];
            normA += Math.pow(embed1[i], 2);
            normB += Math.pow(embed2[i], 2);
        }
        return dotProduct / (Math.sqrt(normA) * Math.sqrt(normB));
    }

    private static double cosineSimilarity(String embed1, String embed2) {
        List<String> embed1_array = Arrays.asList(embed1.split("\\|"));
        List<String> embed2_array = Arrays.asList(embed2.split("\\|"));
        //each string is a embedding, randomly choose at most 3 pairs from each array to the cosine similarity,
        //then make the average of those embedding
        boolean array1Longer = false;
        if (embed2_array.size() < embed1_array.size()) {
            array1Longer = true;
        }
        int randomTimes = 3;
        if (array1Longer) {
            randomTimes = Math.min(embed2_array.size(), randomTimes);
        } else {
            randomTimes = Math.min(embed1_array.size(), randomTimes);
        }
        double similarity = 0.0;
        while (randomTimes != 0) {
            randomTimes--;
            int index1 = ThreadLocalRandom.current().nextInt(0, embed1_array.size());
            int index2 = ThreadLocalRandom.current().nextInt(0, embed2_array.size());
            // double[] em1 = embed1_array.get(index1).split(" ");
            //ToDo: need to check if the embed is sperated by " "
            double[] em1 = Arrays.stream(embed1_array.get(index1).split(" ")).mapToDouble(Double::parseDouble).toArray();
            double[] em2 = Arrays.stream(embed2_array.get(index2).split(" ")).mapToDouble(Double::parseDouble).toArray();
            similarity += cosineSimilarity_single(em1, em2);
            embed1_array.remove(index1);
            embed2_array.remove(index2);
        }
        //then return the average
        return similarity / randomTimes;
    }

    /**
     * returns how many matches occured between two strings , make it return a number insteadd of boolean in order to
     * do a weighted way later.
     *
     * @param s1
     * @param s2
     * @return
     */
    private static int stringMatchCount(String s1, String s2) {
        int count = 0;
        String[] s1_array = s1.split(",");
        String[] s2_array = s2.split(",");
        List<String> list = Arrays.asList(s2_array);
        //array does has contains method, so change it to a list.
        //return Arrays.stream(s1_array).parallel().anyMatch(Arrays.asList(s2_array)::contains);
        for (String s : s1_array) {
            if (list.contains(s)) {
                count++;
            }
        }
        return count;
    }

    private static boolean stringMatch(String s1, String s2) {
        if (stringMatchCount(s1, s2) > 0)
            return true;
        return false;
    }

    /**
     * it could be link event to event, link event to incidence, and link incidence to incidence,
     * can think row1 as the to link row and row2 is the target row
     */
    private static boolean linkEvent(HashMap<String, String> row1, HashMap<String, String> row2) {

        String embed1 = row1.get("embed").replace("[","").replace("]","");
        String embed2 = row2.get("embed").replace("[","").replace("]","");
        double similarity = cosineSimilarity(embed1, embed2);
        if (similarity < similarity_threshold) {
            return false;
        }

        double totalScore = 0;
        for (String feature : featureList) {
            String currentFeature1 = row1.get(feature);
            String currentFeature2 = row2.get(feature);
            if (stringMatch(currentFeature1, currentFeature2)) {
                totalScore += weightMap.get(feature);
            }
        }

        if (totalScore >= threshold) {
            return true;
        }

        return false;
    }

    /**
     * change a row structure to a Row in this way, can update this hashMap value.
     *
     * @param row
     * @return
     */
    private HashMap<String, String> rowToMap(Row row) {
        HashMap<String, String> map = new HashMap<>();
        for (String feature : featureList) {
            map.put(feature, row.getAs(feature));
        }
        return map;
    }

    private static boolean singleStringInString(String singleString, String str) {
        String[] strList = str.split(",");
        return Arrays.asList(strList).contains(singleString);
    }

    /**
     * return the deduped (union) of string list
     *
     * @param str1
     * @param str2
     * @return
     */
    private static String dedup(String str1, String str2) {
        String[] list1 = str1.split(",");
        String[] list2 = str2.split(",");

        Set<String> set = new HashSet<String>();
        set.addAll(Arrays.asList(list1));
        set.addAll(Arrays.asList(list2));

        return String.join(",", set);
    }

    public Dataset<Row> runLinkage(String dimension, Dataset<Row> events) {

        log.info("Start to run linkage on the dimension: " + dimension);

        KeyValueGroupedDataset<String, Row> groupedEvents = events.groupByKey(new MapFunction<Row, String>() {
            @Override
            public String call(Row row) throws Exception {
                return row.getAs(dimension);
            }
        }, Encoders.bean(String.class));

        log.info("The size of groupedEvents is: " + groupedEvents.count());

        List<HashMap<String, String>> hashmapList = new ArrayList<>();
        Dataset<Row> linkedEvents = groupedEvents.flatMapGroups(new FlatMapGroupsFunction<String, Row, Row>() {
            @Override
            public Iterator<Row> call(String dimension, Iterator<Row> rowIter) throws Exception {

                // todo: @Yan, please provide your implementation here
                List<Row> newRows = new ArrayList<>();
                while (rowIter.hasNext()) {
                    Row oldRow = rowIter.next();
                    hashmapList.add(rowToMap(oldRow));
                }
                //after we construct the hashMapList, we can make the linkage now.

                //step1: randomly choose two element in the arrayList:
                int linkCount = 0;
                //toDo: 10000 is set for test purpose and need to be changed
                while (linkCount < 10000) {
                    linkCount = linkCount + 1;

                    int eventLength = hashmapList.size();
                    int fromIndex = ThreadLocalRandom.current().nextInt(0, eventLength);
                    int toIndex = ThreadLocalRandom.current().nextInt(0, eventLength);

                    HashMap<String, String> event_from = hashmapList.get(fromIndex);
                    HashMap<String, String> event_to = hashmapList.get(toIndex);

                    if (linkEvent(event_from, event_to)) {
                        //if linked, merge the from events to to events, and get rid of the to events.
                        //also need to append embedding
                        for (String feature : featureList) {

                            //embed needs to use seperated delimiter
                            if (feature != "embed") {
                                event_to.put(feature, dedup(event_from.get(feature), event_to.get(feature)));
                            } else {
                                //use | to seperate the embedding
                                event_to.put("embed", event_from.get("embed") + "|" + event_to.get("embed"));
                            }
                            //get rid of the event_from from the list
                            hashmapList.remove(fromIndex);
                        }

                    }
                }

                //transfer the hashmap to Row
                for(HashMap<String, String> hashmap : hashmapList)
                {
                   //get rid of the last ","

                   Row newrow = RowFactory.create(StringUtils.substring(String.join(",", hashmap.values()), 0, -1));
                   newRows.add(newrow);
                }

                return newRows.iterator();
            }
        }, Encoders.bean(Row.class));

        return linkedEvents;
    }
}
