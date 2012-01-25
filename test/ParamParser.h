#ifndef PARAMPARSER_H
#define PARAMPARSER_H

#include <iostream>
namespace rtps
{
    class ParamParser
    {
    public:
        ParamParser(){}
        void readParameterFile(std::istream& is);
    private:
    };
};

#endif