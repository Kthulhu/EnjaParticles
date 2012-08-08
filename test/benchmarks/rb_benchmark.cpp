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
#include <GL/glew.h>
#include "rb_benchmark.h"
#include "../ParamParser.h"
#include "../../rtpslib/RTPS.h"
#include "../../rtpslib/render/SSEffect.h"
#include "../../rtpslib/render/ParticleEffect.h"
#include "../../rtpslib/render/MeshEffect.h"
#include "../../rtpslib/system/ParticleRigidBody.h"
#include "../../rtpslib/system/SPH.h"
#include "../../rtpslib/system/FLOCK.h"
#include "../aiwrapper.h"
#include "../../rtpslib/render/util/stb_image_write.h"

#include <math.h>
#include <sstream>
#include <float.h>

#if defined __APPLE__ || defined(MACOSX)
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
//OpenCL stuff
#endif

using namespace std;
#define NUM_X 3
#define NUM_Y 3
#define NUM_Z 3
namespace rtps
{
    RB_Benchmark::RB_Benchmark(istream& is, string path,GLuint w,GLuint h):TestApplication(path,w,h)
    {
	mass=5.0f;
	initParams(is);
	initScenes();

	std::string name = dynamicMeshes.begin()->first;
	ParticleShape* shape = pShapes[name];
	currentMesh = name;
	float delta = shape->getMaxDim()-shape->getMinDim();
	float4 pos = float4(1.0f, 1.0f, 1.0f, 1.0f);
	float4 vel = float4(0.0f, 0.0f, 0.0f, 0.0f);
	for(int i = 0; i<NUM_X; i++)
	{
	    for(int j = 0; j<NUM_Y; j++)
	    {
	        for(int k = 0; k<NUM_Z; k++)
	        {
		    addRigidBody("rb1",name,pos,vel,mass);
		    pos.z+=delta;
	        }
		pos.y+=delta;
            }
            pos.x+=delta;
	}
    }
    RB_Benchmark::~RB_Benchmark()
    {
    }

    void RB_Benchmark::initScenes()
    {
	loadScene(binaryPath+"/demo_scene_plane.obj");
	loadMeshScene(binaryPath+"/demo_mesh_scene.obj");
    }
   void RB_Benchmark::MouseCallback(int button, int state, int x, int y)
    {
    }
    void RB_Benchmark::MouseMotionCallback(int x, int y)
    {
    }
    void RB_Benchmark::KeyboardCallback(unsigned char key, int x, int y)
    {
    }
    void RB_Benchmark::readParamFile(istream& is, string path)
    {
    ParamParser p;
    vector<RTPSSettings*> sysSettings;
    vector<string> names;
    p.readParameterFile(is,sysSettings ,names );
    for(unsigned int i = 0; i<sysSettings.size(); i++)
    {
        //#ifdef WIN32
        //    sysSettings[i]->SetSetting("rtps_path",".");
        //#else
        //    sysSettings[i]->SetSetting("rtps_path","./bin");
        //#endif
		sysSettings[i]->SetSetting("rtps_path",binaryPath);
        systems[names[i]]=RTPS::generateSystemInstance(sysSettings[i],cli);
        systemRenderType[names[i]] = "Mesh Renderer";
    }
    gridMin = systems["rb1"]->getSettings()->GetSettingAs<float4>("domain_min");
    gridMax = systems["rb1"]->getSettings()->GetSettingAs<float4>("domain_max");
    for(map<std::string,System*>::iterator i = systems.begin(); i!=systems.end(); i++)
    {
        for(map<std::string,System*>::iterator j = systems.begin(); j!=systems.end(); j++)
        {
            if(i==j)
                continue;
            i->second->addInteractionSystem(j->second);
        }
    }
}

    void RB_Benchmark::TimerCallback(int ms)
    {
            if(!paused)
	    {
		glFinish();
		for(map<std::string,System*>::iterator i = systems.begin(); i!=systems.end(); i++)
		{
		    i->second->acquireGLBuffers();
		    i->second->update();
		    i->second->interact();
		}

		for(map<std::string,System*>::iterator i = systems.begin(); i!=systems.end(); i++)
		{
		    i->second->integrate();
		    i->second->postProcess();
		    i->second->releaseGLBuffers();
		}
	    }
	glutPostRedisplay();
    }
};
