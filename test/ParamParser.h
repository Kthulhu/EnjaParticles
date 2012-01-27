#ifndef PARAMPARSER_H
#define PARAMPARSER_H

#include <iostream>
#include <string>
namespace rtps
{
    class ParamParser
    {
    public:
        ParamParser(){}
        void readParameterFile(std::istream& is, vector<RTPSSettings*>& systems, vector<std::string>& names);
    private:
    };
};

#endif