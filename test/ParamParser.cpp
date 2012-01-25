#include "ParamParser.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
namespace rtps
{
    void ParamParser::readParameterFile(std::istream& is)
    {
        // populate tree structure pt
        using boost::property_tree::ptree;
        ptree pt;
        read_xml(is, pt);

        // traverse pt
        BOOST_FOREACH( ptree::value_type const& v, pt.get_child("main") ) {
            std::cout<<v.first<<std::endl;
        }
    }
};