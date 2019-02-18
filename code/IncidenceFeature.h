#ifndef INCIDENCEFEATURE_H_
#define INCIDENCEFEATURE_H_

#include <map>
#include <string>


//to track if the feature turned on or off on each incidence, if incidence feature has a value it means it is turned on, otherwise it means it is turned off.
class IncidenceFeature
{
	public:
		std::map<std::string, std::string> featureMap;
		IncidenceFeature()
		{
			//all the feature should be turned on at the beginning
			//all the names here are the same as the json file, if it has value means it is turned on, if it is "" string, means this feature is turned off
			featureMap["code"] = "";
			featureMap["country_code"] = "";
			featureMap["date8"] = "";
			featureMap["geoname"] = "";
			featureMap["id"] = "";
			featureMap["year"] = "";
			featureMap["latitude"] = "";
			featureMap["longitude"] = "";
			featureMap["src_actor"] = "";
			featureMap["src_agent"] = "";
			featureMap["tgt_actor"] = "";
			featureMap["tgt_agent"] = "";
		}
};

#endif // INCIDENCEFEATURE_H_
