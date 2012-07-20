/****************************************************************************************
* Real-Time Particle System - An OpenCL based Particle system developed to run on modern GPUs. Includes SPH fluid simulations.
* version 1.0, September 14th 2011
*
* Copyright (C) 2011 Ian Johnson, Andrew Young, Gordon Erlebacher, Myrna Merced, Evan Bollig
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
****************************************************************************************/


#ifndef RTPS_RTPSSETTINGS_H_INCLUDED
#define RTPS_RTPSSETTINGS_H_INCLUDED

#include <string>
#include <map>
#include <sstream>
#include <iostream>

#include "util.h"
#include "rtps_common.h"

//#include "domain/Domain.h"

namespace rtps
{
    //next largest power of 2. hack required for BitonicSort
    unsigned int nlpo2(register unsigned int x);

    class RTPS_EXPORT RTPSSettings
    {
    public:

        RTPSSettings();
        //without this, windows was crashing with a ValidHeapPointer
        //assertion error. Indicates the heap may be corrupted by
        //something in here
        ~RTPSSettings();

        bool hasChanged() { return changed; }
        //for now we are assuming only one consumer (one system using the settings)
        void updated() { changed = false; }
        void printSettings();

        //----------------------------------------------------------------------
        // Return the value associate with KEY as the specified template parameter type
        // e.g.,
        //  int i = SPHSettings.GetSettingAs<int>("key");
        //  double d = SPHSettings.GetSettingAs<double>("key2");
        //  string s = SPHSettings.GetSettingAs<string>("key3");
        template <typename RT>
        RT GetSettingAs(std::string key, std::string defaultval = "0")
        {
            if (settings.find(key) == settings.end())
            {
                //dout<<"key = "<<key<< " was not found! Using default value "<<defaultval<<std::endl;
                RT ret = ss_typecast<RT>(defaultval);
                return ret;
            }
            return ss_typecast<RT>(settings[key]);
        }

		//----------------------------------------------------------------------
        template <typename RT>
        void SetSetting(std::string key, RT value) {
            // TODO: change to stringstream for any type of input that is cast as string
            std::ostringstream oss;
            oss << value;
            settings[key] = oss.str();
            //dout << "setting: " << key << " | " << value << std::endl;//printf("setting: %s %s\n", settings[key].c_str());
            changed = true;
        }

		//----------------------------------------------------------------------
        bool Exists(std::string key);

    private:
        std::map<std::string, std::string> settings;
        bool changed;

        // This routine is adapted from post on GameDev:
        // http://www.gamedev.net/community/forums/topic.asp?topic_id=190991
        // Should be safer to use this than atoi. Performs worse, but our
        // hotspot is not this part of the code.
        template<typename RT, typename _CharT, typename _Traits , typename _Alloc >
        RT ss_typecast( const std::basic_string< _CharT, _Traits, _Alloc >& the_string )
        {
            std::basic_istringstream< _CharT, _Traits, _Alloc > temp_ss(the_string);
            RT num;
            temp_ss >> num;
            //temp_ss.operator>>(num);
            return num;
        }
    };

};

#endif
