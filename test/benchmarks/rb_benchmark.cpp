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

#include <stdlib.h>
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
<<<<<<< HEAD
#define NUM_X 20
#define NUM_Y 30
#define NUM_Z 20
#define SCALE 0.5
=======
#define NUM_X 15
#define NUM_Y 25
#define NUM_Z 15
#define SCALE .5
>>>>>>> d9896014ca57f954b9a0d51318c3db767c1d5b5a
namespace rtps
{
    RB_Benchmark::RB_Benchmark(istream& is, string path,GLuint w,GLuint h, unsigned int maxIterations):TestApplication(path,w,h)
    {
	mass=.1f;
	initParams(is);
	initScenes();
	this->maxIterations=maxIterations;
	iterations=0;
	systemRenderType["rb1"]="Mesh Renderer";

	std::string name = dynamicMeshes.begin()->first;
	dout<<"origshape minDim = "<<pShapes[name]->getMinDim()<<" origshape maxDim = "<<pShapes[name]->getMaxDim()<<endl;
	ParticleShape* shape = createParticleShape("rb1",dynamicMeshes[name],SCALE);
	dout<<"shape minDim = "<<shape->getMinDim()<<" shape maxDim = "<<shape->getMaxDim()<<endl;
	stringstream s;
	s<<name<<SCALE;
	Mesh* scaledMesh=new Mesh(*dynamicMeshes[name]);
	scaledMesh->modelMat[0]*=SCALE;
	scaledMesh->modelMat[5]*=SCALE;
	scaledMesh->modelMat[10]*=SCALE;
	name = s.str();
	pShapes[name]=shape;
	dynamicMeshes[name]=scaledMesh;
	currentMesh = name;
<<<<<<< HEAD
	float delta = shape->getMaxDim()-shape->getMinDim();
	float4 start = gridMin+float4(2.0f, 2.0f, 2.0f, 0.0f);
=======
	float delta = (shape->getMaxDim()-shape->getMinDim())*1.5;
	float4 start = gridMin+float4(2.0f, 3.0f, 2.0f, 0.0f);
>>>>>>> d9896014ca57f954b9a0d51318c3db767c1d5b5a
	float4 pos = start;
	float4 vel = float4(0.0f, 0.0f, 0.0f, 0.0f);
	light->pos.z=25.0f;
	unsigned int count =0;
	for(int i = 0; i<NUM_X; i++)
	{
	    pos.y=start.y;
	    for(int j = 0; j<NUM_Y; j++)
	    {
		pos.z=start.z;
	        for(int k = 0; k<NUM_Z; k++)
	        {
		    addRigidBody("rb1",name,pos,vel,mass);
		    count++;
		    pos.z+=delta;
	        }
		pos.y+=delta;
            }
            pos.x+=delta;
	}
	dout<<"Count = "<<count<<endl;
	view->move(-5.0f,0.0f,-10.0f);
    }
    RB_Benchmark::~RB_Benchmark()
    {
    }

    void RB_Benchmark::initScenes()
    {
<<<<<<< HEAD
	//loadScene(binaryPath+"/demo_scene_plane.obj");
=======
>>>>>>> d9896014ca57f954b9a0d51318c3db767c1d5b5a
	loadScene(binaryPath+"/demo_scene_big_box.obj");
	loadMeshScene(binaryPath+"/demo_mesh_scene.obj");
    }
   void RB_Benchmark::MouseCallback(int button, int state, int x, int y)
    {
	//TestApplication::MouseCallback(button,state,x,y);
    }
    void RB_Benchmark::MouseMotionCallback(int x, int y)
    {
	//TestApplication::MouseMotionCallback(x,y);
    }
    void RB_Benchmark::KeyboardCallback(unsigned char key, int x, int y)
    {
	//TestApplication::KeyboardCallback(key,x,y);
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
        systemRenderType[names[i]] = "Points";//"Mesh Renderer";
        //systemRenderType[names[i]] = "Mesh Renderer";
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
	if(iterations++>=maxIterations)
	{
                for(map<std::string,System*>::iterator i = systems.begin(); i!=systems.end(); i++)
                {
                    i->second->printTimers();
                }
                for(map<std::string,ParticleEffect*>::iterator i = effects.begin(); i!=effects.end(); i++)
                {
                    i->second->printTimers();
                }
		cout<<"Max iterations reached "<<endl;
		exit(0);
	}
    }
};
