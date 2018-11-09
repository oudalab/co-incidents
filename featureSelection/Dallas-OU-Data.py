import json
import gzip
data = []
count=0
with open('newcsv.csv','a') as output:
    with gzip.open('/Users/yanliang/eventData/yan-virtualenv/document_cluster/terrier-location-text-source-part1.json.gz','rt') as f:
        for line in f:
              #print(line)
            count=count+1;
            if(count>20):
                break;
            try:
                jsonobj=json.loads(line)
                #print(jsonobj)
                csvobj=str(jsonobj['mongo_id'])+","+str(jsonobj["doc_id"])+","+" "+","+str(jsonobj["text"]).rstrip()+","+str(jsonobj["code"])+","\
                +str(jsonobj["src_actor"])+","+str(jsonobj["month"])+","+str(jsonobj["tgt_agent"])+","+str(jsonobj['geo_location']['countryCode'])+\
                ","+str(jsonobj['year'])+","+str(jsonobj['source'])+","+str(jsonobj["date8"])+","+str(jsonobj["src_agent"])+","+str(jsonobj["tgt_actor"])+\
                ","+str(jsonobj["geo_location"]["lat"])+","+str(jsonobj["src_other_agent"])+","+str(jsonobj["quad_class"])+","+str(jsonobj["root_code"])+\
                ","+str(jsonobj["tgt_other_agent"])+","+str(jsonobj["day"])+","+str(jsonobj["target"])+","+str(jsonobj["goldstein"])+\
                ","+str(jsonobj["geo_location"]["location_name"])+","+str(jsonobj["geo_location"]["lon"])+","+str(jsonobj["url"])
                output.write(csvobj+"\n")
                print(csvobj)
            except Exception as e:
                print(e);
