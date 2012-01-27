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


#include "GL/glew.h"
#include <RTPS.h>
//#include "system/Simple.h"
#include <system/SPH.h>
#include <system/FLOCK.h>
#include <system/ParticleRigidBody.h>

#include <string.h>
using namespace std;

namespace rtps
{

    /*RTPS::RTPS()
    {
        cli = new CL();
        cl_managed = true;
        //settings will be the default constructor
        Init();
printf("done with constructor\n");
    }

    RTPS::RTPS(RTPSettings *s)
    {
        cli = new CL();
        cl_managed = true;
        settings = s;
        Init();
        printf("done with constructor\n");
    }

    RTPS::RTPS(RTPSettings *s, CL* _cli)
    {
        cli = _cli;
        cl_managed = false;
        settings = s;
        Init();
printf("done with constructor\n");
    }

    RTPS::~RTPS()
    {
        printf("RTPS destructor\n");
        delete settings;
        delete system;
        if(cl_managed)
        {
            delete cli;
        }
        //delete renderer;
    }*/

    void RTPS::GenerateSystemInstance(RTPSSettings* settings, CL* cli)
    {
        //this should already be done, but in blender its not
        //whats best way to check if stuff like glGenBuffers has been inited?
        //glewInit();

        dout<<"init: settings->system: "<< settings->GetSettingsAs<string>("system")<<endl;
        //TODO choose based on settings
        if (settings->GetSettingsAs<string>("system") == "sph")
        {
            dout<<"*** sph system 1  ***"<<endl;
            //FIXME: I set max gravity sources to 5. This should be configurable.
            system = new SPH(settings,cli);
        }
        else if (settings->GetSettingsAs<string>("system")  == "flock")
        {
            dout<<"flock system"<<endl;
            system = new FLOCK(settings,cli);
        }
        else if (settings->GetSettingsAs<string>("system")  == "rigidbody")
        {
            dout<<"*** particleRigidBody system 1  ***"<<endl;
            system = new ParticleRigidBody(settings,cli);
        }
    }

    /*void RTPS::update()
    {
        //eventually we will support more systems
        //then we will want to iterate over them
        //or have more complex behavior if they interact
        system->update();
    }

    void RTPS::render()
    {
        system->render();
    }

    void RTPS::printTimers()
    {
            system->printTimers();
    }*/
};

