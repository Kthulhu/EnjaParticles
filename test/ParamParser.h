#ifndef PARAMPARSER_H
#define PARAMPARSER_H

#include <iostream>
#include <vector>
#include <string>
#include "../rtpslib/RTPSSettings.h"
namespace rtps
{
    class ParamParser
    {
    public:
        ParamParser(){}
        void readParameterFile(std::istream& is, std::vector<RTPSSettings*>& systems, std::vector<std::string>& names);
    private:
    };
};

#endif