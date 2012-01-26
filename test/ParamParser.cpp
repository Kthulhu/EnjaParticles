#include "ParamParser.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <string>
#include "structs.h"
#include "RTPSettings.h"

using namespace std;
namespace rtps
{
    void ParamParser::readParameterFile(istream& is)
    {
        // populate tree structure pt
        using boost::property_tree::ptree;
        ptree pt;
        read_xml(is, pt);

        // traverse pt
        BOOST_FOREACH( ptree::value_type const& v, pt.get_child("main") ) 
        {
            cout<<v.first<<endl;
            if(v.first=="system")
            {
                string sysType,sysName;
                RTPSettings settings;
                BOOST_FOREACH( ptree::value_type const& v2, v.second) 
                {
                    cout<<v2.first<<endl;
                    if(v2.first=="<xmlattr>")
                    {
                        //cout<<"type = "<<<<endl;
                        //cout<<"name = "<<<<endl;
                        sysType=v2.second.get<string>("type");
                        sysName=v2.second.get<string>("name");
                    }
                    else if(v2.first=="parameters")
                    {
                        BOOST_FOREACH( ptree::value_type const& v3, v2.second) 
                        {
                             if(v3.first=="<xmlattr>")
                             {
                                 cout<<"parameter type = "<<v3.second.get<string>("type")<<endl;
                                 continue;
                             }
                             cout<<v3.first;
                             cout<<" name = "<<v3.second.get<string>("<xmlattr>.name","no name");
                             string name = v3.second.get<string>("<xmlattr>.name","no name");
                             cout<<" type = "<<v3.second.get<string>("<xmlattr>.type","string")<<endl;
                             string pType=v3.second.get<string>("<xmlattr>.type","string");
                             settings.SetSetting(name,v3.second.get_value<string>());
                             /*if(pType=="float")
                             {
                                 float val=v3.second.get_value<float>();
                                 //cout<<"I'm a float"<<endl;
                                 //cout<<" value = "<<v3.second.get_value<string>()<<endl;
                             }
                             else if(pType=="float4")
                             {
                                 float4 val(v3.second.get_value<string>());
                                 //val.print("value");
                                 //cout<<" value = "<<v3.second.get_value<string>()<<endl;
                             }
                             else if(pType=="unsigned int")
                             {
                                 unsigned int val=v3.second.get_value<unsigned int>();
                                 //cout<<"I'm a unsigned int"<<endl;
                                 //cout<<" value = "<<v3.second.get_value<string>()<<endl;
                             }
                             else if(pType=="bool")
                             {
                                 bool val=v3.second.get_value<bool>();
                                 //cout<<"I'm a bool"<<endl;
                             }
                             else
                             {
                                 string val = v3.second.get_value<string>();
                                 //cout<<"I'm a string"<<endl;
                             }*/
                        }
                    }
                }
                settings.printSettings();//GetSettingAs<float4>("gravity").print("gravity");
            }
        }
    }
};
