#ifndef SUBINCIDENCE_H_
#define SUBINCIDENCE_H_

#include <string>
#include <vector>


class SubIncidence
{
public:
    //it hard to generate guid in c++, so maybe Subincidence don't need a guid
    std::string sub_id;
    std::string inci_id;
    std::vector<std::string> sentencesid;
    /*****list of features that subincidence care about*/;
    SubIncidence(std::string subid, std::string inciid): sub_id(subid), inci_id(inciid) {}
};


#endif // SUBINCIDENCE_H_
