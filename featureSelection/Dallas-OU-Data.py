import json
import gzip
import sys

count=0
index=str(sys.argv[1]);
with open('newtsv-part'+index+'.tsv','a') as output:
    with gzip.open('terrier-location-text-source-part'+index+'.json.gz','rt') as f:
        for line in f:
              #print(line)
            #count=count+1;
            if(len(line.strip()) == 0 or line in ['\n', '\r\n']):
                continue;
            try:
                jsonobj=json.loads(line)
                #print(jsonobj)
                code=""
                countryCode=""
                lat=""
                lon=""
                location_name=""
                src_actor=""
                tgt_agent=""
                mongo_id=""
                doc_id=""
                text=""
                month=""
                year=""
                source=""
                date8=""
                src_agent=""
                tgt_actors=""
                src_other_agent=""
                root_code=""
                quad_class=""
                tgt_other_agent=""
                day=""
                target=""
                goldstein=""
                url=""
                if "code" in jsonobj:
                    code=str(jsonobj["code"])
                if "geolocation" in jsonobj:
                    lat=str(jsonobj["geo_location"]["lat"])
                    location_name=str(jsonobj["geo_location"]["location_name"])
                    countryCode=str(jsonobj['geo_location']['countryCode'])
                    lon=str(jsonobj["geo_location"]["lon"])
                if "src_actor" in jsonobj:
                    src_actor=str(jsonobj["src_actor"])
                if "tgt_agent" in jsonobj:
                    tgt_agent=str(jsonobj["tgt_agent"])
                if "mongo_id" in jsonobj:
                    mongo_id=str(jsonobj['mongo_id'])
                if "doc_id" in jsonobj:
                    doc_id=str(jsonobj["doc_id"])
                if "text" in jsonobj:
                    text=str(jsonobj["text"]).rstrip()
                if "month" in jsonobj:
                    month=str(jsonobj["month"])
                if "year" in jsonobj:
                    year=str(jsonobj['year'])
                if "source" in jsonobj:
                    source=str(jsonobj['source'])
                if "date8" in jsonobj:
                    date8=str(jsonobj["date8"])
                if "src_agent" in jsonobj:
                    src_agent=str(jsonobj["src_agent"])
                if "tgt_actor" in jsonobj:
                    tgt_actor=str(jsonobj["tgt_actor"])
                if "scr_other_agent" in jsonobj:
                    src_other_agent=str(jsonobj["src_other_agent"])
                if "root_code" in jsonobj:
                    root_code=str(jsonobj["root_code"])
                if "quad_class" in jsonobj:
                    quad_class=str(jsonobj["quad_class"])
                if "tgt_other_agent" in jsonobj:
                    tgt_other_agent=str(jsonobj["tgt_other_agent"])
                if "day" in jsonobj:
                    day=str(jsonobj["day"])
                if "target" in jsonobj:
                    target=str(jsonobj["target"])
                if "goldstein" in jsonobj:
                    goldstein=str(jsonobj["goldstein"])
                if "url" in jsonobj:
                    url=str(jsonobj["url"])
                tsvobj=mongo_id+"\t"+doc_id+"\t"+" "+"\t"+code+"\t"\
                +src_actor+"\t"+month+"\t"+tgt_agent+"\t"+countryCode+\
                "\t"+year+"\t"+source+"\t"+date8+"\t"+ src_agent+"\t"+tgt_actor+\
                "\t"+lat+"\t"+src_other_agent+"\t"+quad_class+"\t"+root_code+\
                "\t"+tgt_other_agent+"\t"+day+"\t"+target+"\t"+goldstein+\
                "\t"+location_name+"\t"+lon+"\t"+url+"\t"+text
                output.write(tsvobj+"\n")
                #print(tsvobj.split(",")[0])
            except Exception as e:
                print(e);
