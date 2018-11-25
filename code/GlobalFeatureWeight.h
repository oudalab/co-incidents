#ifndef GLOBALFEATUREWEIGHT_H_
#define GLOBALFEATUREWEIGHT_H_

#include <map>
#include <string>

class GlobalFeatureWeight
{
	public:
        std::map<std::string, int> featureWeight;
		GlobalFeatureWeight()
		{
			featureWeight["code"] = 1;
			featureWeight["root_code"] = 1;
			featureWeight["country_code"] = 1;
			featureWeight["date8"] = 1;
			featureWeight["geoname"] = 1;
			featureWeight["id"] = 1;
			featureWeight["year"] = 1;
			featureWeight["latitude"] = 1;
			featureWeight["longitude"] = 1;
			featureWeight["src_actor"] = 1;
			featureWeight["src_agent"] = 1;
			featureWeight["tgt_actor"] = 1;
			featureWeight["tgt_agent"] = 1;
			featureWeight["tgt_year"] = 1;
		}
};

#endif // GLOBALFEATUREWEIGHT_H_
