#ifndef SENVENCEFEATUREVALUE_H_
#define SENVENCEFEATUREVALUE_H_

#include <string>

using namespace std;

class SentenceFeatureValue
{
public:
    std::string code;
    std::string rootcode;
    double latitude;
    double longitude;
    std::string geoname;
    std::string date8;
    std::string id;
    std::string year;
    std::string src_actor;
    std::string src_agent;
    std::string tgt_actor;
    std::string tgt_agent;
    std::string month;
    std::string day;
    int *embed;
    //this will be the index in the global sentence array
    int index;
    
    SentenceFeatureValue(string code1, std::string rootcode1, std::string date81,  std::string id1, std::string year1, std::string src_actor1, std::string src_agent1, std::string tgt_actor1, std::string tgt_agent1, std::string month1, std::string day1, int *embed1, int index1, double latitude, double longitude, std::string geoname)
    {
        code = code1;
        rootcode = rootcode1;
        date8 = date81;
        id = id1;
        year = year1;
        src_actor = src_actor1;
        src_agent = src_agent1;
        tgt_actor = tgt_actor1;
        tgt_agent = tgt_agent1;
        month = month1;
        day = day1;
        embed = embed1;
        index = index1;
        latitude=latitude;
        longitude=longitude;
        geoname=geoname;
        trimall();
    };
private:
    void trimall()
    {
        rtrim(code);
        rtrim(rootcode);
        rtrim(date8);
        rtrim(id);
        rtrim(year);
        rtrim(month);
        rtrim(day);
        rtrim(src_actor);
        rtrim(src_agent);
        rtrim(tgt_actor);
        rtrim(tgt_agent);
        rtrim(geoname);
    }
private:
    void rtrim(std::string &s)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(),
                    [](int ch) { 
                        return !std::isspace(ch);
                    }).base(), s.end());
    }
};

#endif // SENVENCEFEATUREVALUE_H_

