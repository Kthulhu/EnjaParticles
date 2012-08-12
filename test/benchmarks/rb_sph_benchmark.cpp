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
#include "rb_sph_benchmark.h"
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
#define NUM 15 
//#define SCALE 0.4
namespace rtps
{
    RBSPHBenchmark::RBSPHBenchmark(istream& is, string path,GLuint w,GLuint h, unsigned int maxIterations):TestApplication(path,w,h)
    {
	//mass=0.5f;
	mass=1.0f;
	initParams(is);
	initScenes();
	this->maxIterations=maxIterations;
	iterations=0;

	systemRenderType["rb1"]="Mesh Renderer";
	//systemRenderType["water"]="Screen Space";
	unsigned int nn = systems["water"]->getSettings()->GetSettingAs<unsigned int>("max_num_particles");
	float4 col1 = float4(0.05f, 0.15f, .8f, 0.1f);
	systems["water"]->addBox(nn, gridMin+float4(2.5f,2.5f,2.5f,1.0f), gridMax-float4(2.5f,0.25*gridMax.y+2.5f,2.5f,1.0f), false,col1);

	view->move(-5.0f,5.0f,-15.0f);
	view->rotate(30.0f,0.0f);

    }

    void RBSPHBenchmark::dropRigidBodies()
    {
	dout<<"rb spacing "<<systems["rb1"]->getSpacing()<<endl;
	std::string name = dynamicMeshes.begin()->first;
	float scale = pShapes[name]->getMaxDim()-pShapes[name]->getMinDim();
	float4 start = gridMin+float4(2.0f, 10.0f, 2.0f, 0.0f);
	float4 distance = gridMax-start;
	distance.y+=50.0f;
	float m = std::min(distance.x,distance.y);
	m = std::min(m,distance.z);
	//scale =(m/NUM)/scale;
	scale = 1.5f; 
	float4 pos = start;
	//dout<<"origshape minDim = "<<pShapes[name]->getMinDim()<<" origshape maxDim = "<<pShapes[name]->getMaxDim()<<endl;
	ParticleShape* shape = createParticleShape("rb1",dynamicMeshes[name],scale);
	dout<<"shape minDim = "<<shape->getMinDim()<<" shape maxDim = "<<shape->getMaxDim()<<endl;
	stringstream s;
	s<<name<<scale;
	Mesh* scaledMesh=new Mesh(*dynamicMeshes[name]);
	scaledMesh->modelMat[0]*=scale;
	scaledMesh->modelMat[5]*=scale;
	scaledMesh->modelMat[10]*=scale;
	name = s.str();
	pShapes[name]=shape;
	dynamicMeshes[name]=scaledMesh;
	currentMesh = name;
	float delta = (shape->getMaxDim()-shape->getMinDim())*1.5;
	float4 vel = float4(0.0f, 0.0f, 0.0f, 0.0f);
	unsigned int count = 0;
	for(int i = 0; i<NUM; i++)
	{
	    pos.y=start.y;
	    for(int j = 0; j<NUM; j++)
	    {
		pos.z=start.z;
	        for(int k = 0; k<NUM; k++)
	        {
		    addRigidBody("rb1",name,pos,vel,mass);
		    count++;
		    pos.z+=delta;
		    if(pos.z>gridMax.z)
		    {
			dout<<"z num = "<<k<<endl;
			break;
		    }
		    else if(systems["rb1"]->getNum()>(unsigned int)(systems["rb1"]->getSettings()->GetSettingAs<unsigned int>("max_num_particles")*0.98))
		    {
			i=NUM;j=NUM;k=NUM;break;
		    }
	        }
		pos.y+=delta;
	        //if(pos.y>gridMax.y+50.0f)
		//{
		//   dout<<"y num = "<<j<<endl;
		//   break;
		//}
            }
            pos.x+=delta;
	    if(pos.x>gridMax.x)
	    {
	       dout<<"x num = "<<i<<endl;
	       break;
	    }
	}
	dout<<"count = "<<count<<endl;

    }
    RBSPHBenchmark::~RBSPHBenchmark()
    {
    }

    void RBSPHBenchmark::initScenes()
    {
	loadScene(binaryPath+"/demo_scene_big_box.obj");
	loadMeshScene(binaryPath+"/demo_mesh_scene.obj");
	//loadMeshScene(binaryPath+"/demo_mesh_sphere_scene.obj");
    }
   void RBSPHBenchmark::MouseCallback(int button, int state, int x, int y)
    {
	//TestApplication::MouseCallback(button,state,x,y);
    }
    void RBSPHBenchmark::MouseMotionCallback(int x, int y)
    {
	//TestApplication::MouseMotionCallback(x,y);
    }
    void RBSPHBenchmark::KeyboardCallback(unsigned char key, int x, int y)
    {
	//TestApplication::KeyboardCallback(key,x,y);
    }
    void RBSPHBenchmark::TimerCallback(int ms)
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
    if(iterations==300)// || iterations==400|| iterations==500|| iterations==600)
    {
	dropRigidBodies();
    }
    }
};
