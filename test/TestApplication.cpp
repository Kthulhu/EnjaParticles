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
#include "TestApplication.h"
#include "ParamParser.h"
#include "../rtpslib/RTPS.h"
#include "../rtpslib/render/SSEffect.h"
#include "../rtpslib/render/ParticleEffect.h"
#include "../rtpslib/render/MeshEffect.h"
#include "../rtpslib/system/ParticleRigidBody.h"
#include "../rtpslib/system/SPH.h"
#include "../rtpslib/system/FLOCK.h"
#include "aiwrapper.h"
#include "../rtpslib/render/util/stb_image_write.h"

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

namespace rtps
{

const GLfloat fSkyDist = 100.0;

const GLfloat skyBox[] = { fSkyDist,-fSkyDist,-fSkyDist,
                           -fSkyDist,-fSkyDist,-fSkyDist,
                           -fSkyDist,fSkyDist,-fSkyDist,
                           fSkyDist,fSkyDist,-fSkyDist,

                           fSkyDist, -fSkyDist, fSkyDist,
                           fSkyDist, -fSkyDist, -fSkyDist,
                           fSkyDist, fSkyDist, -fSkyDist,
                           fSkyDist, fSkyDist, fSkyDist,

                           -fSkyDist,-fSkyDist,fSkyDist,
                           fSkyDist,-fSkyDist,fSkyDist,
                           fSkyDist,fSkyDist,fSkyDist,
                           -fSkyDist,fSkyDist,fSkyDist,

                           -fSkyDist,-fSkyDist,-fSkyDist,
                           -fSkyDist,-fSkyDist,fSkyDist,
                           -fSkyDist,fSkyDist,fSkyDist,
                           -fSkyDist,fSkyDist,-fSkyDist,

                           -fSkyDist,fSkyDist,-fSkyDist,
                           -fSkyDist,fSkyDist,fSkyDist,
                           fSkyDist,fSkyDist,fSkyDist,
                           fSkyDist,fSkyDist,-fSkyDist,

                           -fSkyDist,-fSkyDist, -fSkyDist,
                           -fSkyDist,-fSkyDist,fSkyDist,
                           fSkyDist,-fSkyDist,fSkyDist,
                           fSkyDist,-fSkyDist,-fSkyDist};


const GLfloat skyBoxTex[] = { 1.f, 0.f,0.f,// 1.f,0.f,0.f,
                           0.f,0.f,0.f,//0.f,0.f,0.f,
                           0.f,1.f,0.f,//0.f,1.f,0.f,
                           1.f,1.f,0.f,//1.f,1.f,0.f,

                           1.f,0.f,1.f,//1.f, 0.f, 1.f,
                           1.f,0.f,0.f,//1.f, 0.f, 0.f,
                           1.f,1.f,0.f,//1.f, 1.f, 0.f,
                           1.f, 1.f, 1.f,

                           0.f,0.f,1.f,
                           1.f,0.f,1.f,
                           1.f,1.f,1.f,
                           0.f,1.f,1.f,

                           0.f,0.f,0.f,
                           0.f,0.f,1.f,
                           0.f,1.f,1.f,
                           0.f,1.f,0.f,

                           0.f,1.f,0.f,
                           0.f,1.f,1.f,
                           1.f,1.f,1.f,
                           1.f,1.f,0.f,

                           0.f,0.f, 0.f,
                           0.f,0.f,1.f,
                           1.f,0.f,1.f,
                           1.f,0.f,0.f};
    TestApplication::TestApplication( string path,GLuint w,GLuint h)
    {
	binaryPath=path;
	mass=0.01f;
	sizeScale=1.0f;

    windowWidth=w;
    windowHeight=h;
    cout<<"width = "<<w<<" height = "<<h<<endl;
	//renderVelocity=true;
	renderVelocity=false;
	paused=false;
	scene=new AIWrapper();
	dynamicMeshScene=new AIWrapper();
	renderMovie=false;
	frameCounter=0;
	view=NULL;
	light=NULL;
	lib=NULL;
	skyboxVBO=0;
	skyboxTexVBO=0;
	glewInit();
	cli = new CL();
	glViewport(0, 0, w, h);

	stereoscopic = false;
	//view = new Camera(float3(5.0f,5.0f,15.0f),65.0,0.3,100.0,width(),height());
	 view = new Camera(float3(5.0f,5.0f,15.0f),65.0,0.3,500.0,w,h);
	//view = new Camera(float3(0.0f,0.0f,0.0f),65.0f,0.3f,100.0f,width(),height());
	//view = new Camera(float3(-105.0f,-105.0f,-105.0f),65.0f,0.3f,1000.0f,width(),height());
	//the models are all in different coordinate systems...
	//view->rotate(-90.f,0.0f);
	view->setMoveSpeed(2.f);
	view->setRotateSpeed(2.f);

	glGenBuffers(1,&skyboxVBO);
	glBindBuffer(GL_ARRAY_BUFFER,skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER,24*3*sizeof(float),skyBox, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER,0);


	glGenBuffers(1,&skyboxTexVBO);
	glBindBuffer(GL_ARRAY_BUFFER,skyboxTexVBO);
	glBufferData(GL_ARRAY_BUFFER,24*3*sizeof(float),skyBoxTex, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER,0);
    glGenFramebuffersEXT(1,&sceneFBO);
    glEnable(GL_TEXTURE_2D);
    createSceneTextures();
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,sceneFBO);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,sceneTex[0],0);

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,sceneTex[1],0);

    dout<<"sceneFBO = "<<sceneFBO<<" status complete? "<<((glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE)?"yes":"no")<<" "<<glCheckFramebufferStatus(GL_FRAMEBUFFER)<<endl;
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
    glDisable(GL_TEXTURE_2D);

	


	light = new Light();
        light->diffuse.x=1.0;light->diffuse.y=1.0;light->diffuse.z=1.0;
        light->ambient.x=0.2;light->ambient.y=0.2;light->ambient.z=0.2;
        light->specular.x=1.0;light->specular.y=1.0;light->specular.z=1.0;
        //light->pos.x=-0.5f; light->pos.y=1.5f; light->pos.z=5.0f;
        //light->pos.x=5.0f; light->pos.y=10.0f; light->pos.z=-5.0f;
        light->pos.x=5.0f; light->pos.y=10.0f; light->pos.z=-5.0f;

	environTex = RenderUtils::loadCubemapTexture(binaryPath+"/cubemaps/");

	lib = new ShaderLibrary();
        lib->initializeShaders(binaryPath+"/shaders");
        effects["Points"]=new ParticleEffect(lib,width(),height());
        effects["Screen Space"]=new SSEffect(lib,width(),height());
        //FIXME: Need to find an elegant solution to handling mesh effects
        meshRenderer= new MeshEffect(lib,width(),height());
        effects["Mesh Renderer"]= meshRenderer;

	//FIXME: Need to find an elegant solution to handling mesh effects
	meshRenderer= (MeshEffect*)effects["Mesh Renderer"];//new MeshEffect(lib,width(),height(),20.0f,false);
    }

void TestApplication::initParams(istream& is)
{
	readParamFile(is,binaryPath);
}
void TestApplication::initScenes()
{
	loadScene(binaryPath+"/demo_scene1.obj");
	loadMeshScene(binaryPath+"/demo_mesh_scene.obj");
}
void TestApplication::initGL()
    {

	glClearColor(0.6f,0.6f,0.6f,1.0f);
	for(std::map<std::string,Shader>::iterator i = lib->shaders.begin(); i!=lib->shaders.end(); i++)
    {
        glUseProgram(i->second.getProgram());
        GLint location = glGetUniformLocation(i->second.getProgram(),"viewMatrix");
        if(location!=-1)
            glUniformMatrix4fv(location,1,GL_FALSE,view->getViewMatrix().m);
            //glUniformMatrix4fv(location,1,GL_FALSE,view->getInverseViewMatrix().m);
        //dout<<i->first<<" viewMatrixLocation = "<<location<<endl;
        location = glGetUniformLocation(i->second.getProgram(),"projectionMatrix");
        if(location!=-1)
            //glUniformMatrix4fv(location,1,GL_FALSE,projectionMatrix.m);
            glUniformMatrix4fv(location,1,GL_FALSE,view->getProjectionMatrix().m);
        //dout<<i->first<<" projectionMatrixLocation = "<<location<<endl;
        location = glGetUniformLocation(i->second.getProgram(),"inverseViewMatrix");
        if(location!=-1)
            glUniformMatrix4fv(location,1,GL_FALSE,view->getInverseViewMatrix().m);
            //glUniformMatrix4fv(location,1,GL_FALSE,view->getViewMatrix().m);
        //dout<<i->first<<" inverseViewMatrixLocation = "<<location<<endl;
        location = glGetUniformLocation(i->second.getProgram(),"inverseProjectionMatrix");
        if(location!=-1)
            glUniformMatrix4fv(location,1,GL_FALSE,view->getInverseProjectionMatrix().m);
        //dout<<i->first<<" inverseProjectionMatrixLocation = "<<location<<endl;
        location = glGetUniformLocation(i->second.getProgram(),"normalMatrix");
        if(location!=-1)
            //glUniformMatrix4fv(location,1,GL_FALSE,view->getViewMatrix().m);
            glUniformMatrix4fv(location,1,GL_TRUE,view->getInverseViewMatrix().m);
        location = glGetUniformLocation(i->second.getProgram(),"near");
        if(location!=-1)
            glUniform1f(location,view->getNearClip());
        location = glGetUniformLocation(i->second.getProgram(),"far");
        if(location!=-1)
            glUniform1f(location,view->getFarClip());
    }
    //cout<<"near clip = "<<view->getNearClip()<<" far clip = "<<view->getFarClip()<<endl;
    GLuint program = lib->shaders["sphereShader"].getProgram();
    glUseProgram(program);
    glUniform1f( glGetUniformLocation(program, "pointScale"), ((float)width()) / tanf(view->getFOV()* (0.5f * PIOVER180)));
    program = lib->shaders["sphereThicknessShader"].getProgram();
    glUseProgram(program);
    glUniform1f( glGetUniformLocation(program, "pointScale"), ((float)width()) / tanf(view->getFOV()* (0.5f * PIOVER180)));
    program = lib->shaders["skybox"].getProgram();
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "skyboxCubeSampler"), 0);
    glUseProgram(0);
    }

void TestApplication::ResizeWindowCallback(int width, int height)
    {
        //avoid height = 0 this will cause divide by zero when calculating aspect ratio
        if (height==0)
        {
            height=1;
        }
	glViewport(0, 0, width, height);
	for(map<std::string,ParticleEffect*>::iterator i = effects.begin(); i!=effects.end(); i++)
	{
		i->second->setWindowDimensions(width,height);
	}
	view->setWidth(width);
	view->setHeight(height);
	windowWidth=width;
	windowHeight=height;
	//set the projection and view matricies for all shaders.
	for(std::map<std::string,Shader>::iterator i = lib->shaders.begin(); i!=lib->shaders.end(); i++)
	{
	 glUseProgram(i->second.getProgram());
	 GLint location = glGetUniformLocation(i->second.getProgram(),"projectionMatrix");
	 if(location!=-1)
	     glUniformMatrix4fv(location,1,GL_FALSE,view->getProjectionMatrix().m);
	 location = glGetUniformLocation(i->second.getProgram(),"inverseProjectionMatrix");
	 if(location!=-1)
	     glUniformMatrix4fv(location,1,GL_FALSE,view->getInverseProjectionMatrix().m);
	}
        createSceneTextures();
	view->rotate(0.000001f,0.000001f);
        glutPostRedisplay();
    }
   void TestApplication::RenderCallback()
    {

        //float time = elapsedTimer->restart()/1000.f;
	float time = 0.033f;
        //send new view matrix to the shaders if the camera was actually updated
        if(view->tick(time))
        {
            cameraChanged();
        }
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

#if 1 
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,sceneFBO);
        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,sceneTex[0],0);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,sceneTex[1],0);
#endif

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



#if 1
        glEnable(GL_DEPTH_TEST);

        glEnable(GL_MULTISAMPLE_EXT);
        //display static opaque objects
        display(false);
#if 1
#if 0
        if(systems.find(std::string("rb1"))!=systems.end())
        {
            //for debugging only!!
            ParticleRigidBody* rbsys = (ParticleRigidBody*)systems[std::string("rb1")];
            effects["Points"]->render(rbsys->getStaticVBO(),rbsys->getColVBO(),rbsys->getStaticNum(),rbsys->getSettings(),light,NULL,rbsys->getSpacing(),sceneTex[0],sceneTex[1], sceneFBO);
        }
#endif
        for(map<std::string,System*>::iterator i = systems.begin(); i!=systems.end(); i++)
        {
            RTPSSettings* settings = i->second->getSettings();
            if(settings->GetSettingAs<bool>("render_velocity","0"))
            {
                effects[systemRenderType[i->first]]->renderVector(i->second->getPosVBO(),i->second->getVelocityVBO(),i->second->getNum(),settings->GetSettingAs<float>("velocity_scale","1.0"));
            }
            if(systemRenderType[i->first]!="Mesh Renderer")
            {
                effects[systemRenderType[i->first]]->render(i->second->getPosVBO(),i->second->getColVBO(),i->second->getNum(),settings,light,NULL,i->second->getSpacing(),sceneTex[0],sceneTex[1], sceneFBO);
            }
            else
            {
                ParticleRigidBody* rbsys = reinterpret_cast<ParticleRigidBody*>(systems[i->first]);
                if(rbsys)
                {
                    meshRenderer->renderInstanced(dynamicMeshes[currentMesh],rbsys->getComPosVBO(),rbsys->getComRotationVBO(),rbsys->getNum(),light);
                }
            }
        }
        //display static transparent objects
        display(true);
#endif
#else
        meshRenderer->render(dynamicMeshs["dynamicShape1"],light);
        glColor4f(0.1f,0.2f,0.4f,1.0f);
        displayShape(pShapes["dynamicShape1"],float3(5.0f,3.f,1.0f),systems["water"]->getSpacing());
        displayShape(pShapes["dynamicShape11"],float3(8.0f,3.f,1.0f),systems["water"]->getSpacing()/2.0f);
        displayShape(pShapes["dynamicShape112"],float3(11.0f,3.f,1.0f),systems["water"]->getSpacing()/4.0f);
        displayShape(pShapes["dynamicShape1123"],float3(14.0f,3.f,1.0f),systems["water"]->getSpacing()/8.0f);
#endif
        //glDisable(GL_DEPTH_TEST);
        //glDisable(GL_MULTISAMPLE_EXT);
#if 1 
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT,sceneFBO);
	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
        glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT,0);
        glDrawBuffer(GL_BACK);

        glBlitFramebufferEXT( 0, 0, width() , height(),
                                          0, 0, width() , height(),
                                          GL_COLOR_BUFFER_BIT, GL_LINEAR );
#endif

        glPopAttrib();
        glPopClientAttrib();
        if(renderMovie)
        {
            writeMovieFrame("image","./frames/");
            frameCounter++;
        }

        glutSwapBuffers();

    }
    TestApplication::~TestApplication()
    {
	    for(map<std::string,System*>::iterator i = systems.begin(); i!=systems.end(); i++)
	    {
		delete i->second;
	    }
	    for(map<std::string,ParticleEffect*>::iterator i = effects.begin(); i!=effects.end(); i++)
	    {
		delete i->second;
	    }
	    for(map<std::string,ParticleShape*>::iterator i = pShapes.begin(); i!=pShapes.end(); i++)
	    {
		delete i->second;
	    }
	    for(map<std::string,Mesh*>::iterator i = meshes.begin(); i!=meshes.end(); i++)
	    {
		delete i->second;
	    }
	    //delete meshRenderer;
	    delete dynamicMeshScene;
	    delete scene;
	    delete cli;
	    delete lib;
	    delete view;
	    delete light;
    }

   void TestApplication::MouseCallback(int button, int state, int x, int y)
    {
        if (state == GLUT_DOWN)
        {
            mouseButtons |= 1<<button;
        }
        else if (state == GLUT_UP)
        {
            mouseButtons = 0;
        }

        mousePos.x = x;
        mousePos.y = y;
    }
    void TestApplication::MouseMotionCallback(int x, int y)
    {
        float dx, dy;
        dx = x - mousePos.x;
        dy = y - mousePos.y;

        if (mouseButtons & 4)
        {
            view->rotate(dy,dx);
        }

        mousePos.x = x;
        mousePos.y = y;

        // set view matrix
        glutPostRedisplay();
    }
    void TestApplication::KeyboardCallback(unsigned char key, int x, int y)
    {
              unsigned int nn=0;
        switch (key)
        {
            case 'm':
                effects["Screen Space"]->writeBuffersToDisk();
                return;
            case ' ':
                paused=!paused;
                return;
            case 'f': //flocks
            {
                nn = systems["flock1"]->getSettings()->GetSettingAs<unsigned int>("max_num_particles")/2;
                systems["flock1"]->addBox(nn, gridMin+float4(0.5f,0.5f,gridMax.z-3.0f,1.0f), gridMin+float4(3.0f,3.0f,gridMax.z-0.5f,1.0f), false);
                return;
            }

            case 'e': //dam break
            {
                nn = systems["water"]->getSettings()->GetSettingAs<unsigned int>("max_num_particles")/2;
                float4 col1 = float4(0.05f, 0.15f, .8f, 0.1f);
                systems["water"]->addBox(nn, gridMin+float4(0.5f,0.5f,0.5f,1.0f), gridMax-float4(0.5f,0.5f,0.5f,1.0f), false,col1);
                //ps2->system->addBox(nn, min, max, false);
                return;
            }
            case 'g':
            {
                //nn = 16384;
                nn = systems["water"]->getSettings()->GetSettingAs<unsigned int>("max_num_particles");
                float4 minCoord = float4(1.0f, 1.0f, 5.0f, 1.0f);
                //float4 max = float4(7.5f, 7.5f, 7.5f, 1.0f);
                float4 maxCoord = float4(9.5f, 9.5f,9.5, 1.0f);
                float4 col1 = float4(0.05f, 0.15f, .8f, 0.1f);
                systems["water"]->addBox(nn, minCoord, maxCoord, false,col1);
                //ps2->system->addBox(nn, min, max, false);
                return;
            }
            case 'p': //print timers
                cout<<"SPH timers:"<<endl;
                systems["water"]->printTimers();
                cout<<"RB timers:"<<endl;
                systems["rb1"]->printTimers();
                return;
            case '\033': // escape quits
            case '\015': // Enter quits
            case 'Q':    // Q quits
            case 'q':    // q (or escape) quits
                // Cleanup up and quit
                //appDestroy();
                return;
            case 'b':
            {
                //matrix is to position the rigidbody at 7,7,7 with no rotations.
                float16 mat(1.0f,0.0f,0.0f,7.0f,
                        0.0f,1.0f,0.0f,7.0f,
                        0.0f,0.0f,1.0f,7.0f,
                        0.0f,0.0f,0.0f,1.0f);
                float4 velocity(0.0f,0.0f,0.0f,0.0f);
                float4 color(1.0f,0.0f,0.0f,1.0f);
                //systems["rb1"]->addParticleShape(bunnyShape->getVoxelTexture(),bunnyShape->getMaxDim(),float4(bunnyShape->getMin(),0.0f),mat,bunnyShape->getVoxelResolution(),velocity,color,mass);
                /*printf("deleting willy nilly\n");
                systems["water"]->testDelete();
                systems["rb1"]->testDelete();*/

                return;
            }
            case 'h':
            {
                //spray hose
                cout<<"about to make hose"<<endl;
                //float4 col1 = float4(0.05f, 0.1f, .2f, 0.1f);
                float4 col1 = float4(0.05f, 0.4f, .8f, 1.0f);
                float4 center = float4(gridMax.x-2.0f, gridMax.y-2.0f,gridMax.z-1.5f,1.0f);
                float4 velocity(-1.25f, -1.25f, -3.0f, 0);
                float radius= 2.0f;
                //sph sets spacing and multiplies by radius value
                systems["water"]->addHose(10000, center, velocity,radius, col1);
                return;
            }
            case 'H':
            {
                //spray hose
                cout<<"about to make hose"<<endl;
                float4 col1 = float4(0.05f, 0.1f, .2f, 0.1f);
                float4 center = float4(gridMax.x-2.0f, gridMax.y-2.0f,gridMax.z-0.5f,1.0f);
                float4 velocity(-1.5f, -1.5f, -4.f, 0.f);
                float radius= 3.0f;
                center = float4(gridMin.x+2.0f, gridMin.y+2.0f,gridMax.z-0.5f,1.0f);
                velocity=float4(1.5f, 0.5f, -.05f, 0.f);
                systems["flock1"]->addHose(50000, center, velocity,radius);
                return;
            }
            case 'n':
                renderMovie=!renderMovie;
                break;
            case '`':
                //stereo_enabled = !stereo_enabled;
                break;
            case 't': //place a cube for collision
                {
                    float4 center=(gridMin+gridMax);
                    center/=2.0f;
                    float innerRadius=1.0f;
                    float outerRadius=4.0f;
                    float thickness=2.0f;
                    systems["water"]->addTorus(systems["water"]->getSettings()->GetSettingAs<unsigned int>("max_num_particles")/2,center,innerRadius,outerRadius,thickness);
                    return;
                }
                //add static floor
            case 'u':
            {
                float4 col1 = float4(0.0f, 0.8f, 0.2f, 1.f);
                float4 size = float4(1.f,1.f,1.f,0.f);
                float4 position = float4(gridMin.x+0.1f, gridMin.y+0.1f,gridMin.z+.1f,1.0f);
                systems["rb1"]->addBox(10000, position, float4(gridMax.x-0.1f,gridMax.y-0.1f,gridMin.z+.5f,1.0f), false, col1,0.0f);
                position = float4(gridMin.x+0.1f, gridMin.y+0.1f,gridMin.z+0.1f,1.0f);
                systems["rb1"]->addBox(10000, position, float4(gridMin.x+0.5f,gridMax.y-0.1f,gridMax.z-.1f,1.0f), false, col1,0.0f);
                position = float4(gridMin.x+0.1f, gridMin.y+0.1f,gridMin.z+0.1f,1.0f);
                systems["rb1"]->addBox(10000, position, float4(gridMax.x-0.1f,gridMin.y+0.5f,gridMax.z-.1f,1.0f), false, col1,0.0f);
                position = float4(gridMax.x-0.5f, gridMin.y+0.1f,gridMin.z+0.1f,1.0f);
                systems["rb1"]->addBox(10000, position, float4(gridMax.x-0.1f,gridMax.y-0.1f,gridMax.z-.1f,1.0f), false, col1,0.0f);
                position = float4(gridMin.x+0.1f, gridMax.y-0.5f,gridMin.z+0.1f,1.0f);
                systems["rb1"]->addBox(10000, position, float4(gridMax.x-0.1f,gridMax.y-0.1f,gridMax.z-.1f,1.0f), false, col1,0.0f);
                return;
            }
            case 'R': //drop a rectangle
                {

                    float4 col1 = float4(0.5f, 0.9f, 0.0f, 1.f);

                    float4 size = float4(1.f,1.f,1.f,0.f);
                    size=size*sizeScale;
                    float4 mid = (gridMax-gridMin);
                    mid = mid/2.0f;
                    mid.z=0.f;
                    float4 position = float4(0.0f, 0.0f,gridMax.z-(size.z/2.f),1.0f);
                    position.x = mid.x-(size.x/2.0f);
                    position.y = mid.y-(size.y/2.0f);
                    position.w = 0.0f;
                    systems["rb1"]->addBox(1000, position, position+size, false, col1,mass);
                    return;
                }
	    case 'j':
		if(systemRenderType["water"]=="Points")
			systemRenderType["water"]="Screen Space";
		else
			systemRenderType["water"]="Points";
		dout<<"system render type for water = "<<systemRenderType["water"]<<endl;
		break;
            case 'r': //drop a ball
            {
                ParticleShape* shape=pShapes["rb1"];
                //float trans = (shape->getMaxDim()+shape->getMinDim())/2.0f;
                //trans +=7.0f;
                //float16 modelview;
                //glMatrixMode(GL_MODELVIEW);
                //glPushMatrix();
                //glLoadIdentity();
                //glRotatef(rotation.x, 1.0, 0.0, 0.0);
                //glTranslatef(trans, trans, trans);
                //glRotatef(-180, 1.0f, 0.0f, 0.0f);
                //glRotatef(-90, 0.0f, 0.0f, 1.0f);
                //glTranslatef(translation.x, translation.z, translation.y);
                //glGetFloatv(GL_MODELVIEW_MATRIX,modelview.m);
                //glPopMatrix();
                //modelview.print("modelview");
                //modelview.transpose();

                //systems["rb1"]->addParticleShape(shape->getVoxelTexture(),shape->getMinDim(),shape->getMaxDim(),modelview,shape->getVoxelResolution(),float4(0.0f,0.0f,0.0f,0.0f),float4(0.0f,0.0f,0.0f,1.0f),mass);

                /*    float4 col1 = float4(0.5, 0.9, 0.0, 1.);
                    float size = 1.0f;
                    size=size*sizeScale;
                    float4 mid = (gridMax-gridMin);
                    mid = mid/2.0f;
                    mid.w = 0.0f;

                    systems["rb1"]->addBall(1000, mid, size,false, col1,mass);*/
                    return;
            }
            case 'w':
                view->move(0.0f,0.0f,1.0f);
                //cameraChanged();
                break;
            case 'a':
                view->move(-1.0f,0.0f,0.0f);
                //cameraChanged();
                break;
            case 's':
                view->move(0.0f,0.0f,-1.0f);
                //cameraChanged();
                break;
            case 'd':
                view->move(1.0f,0.0f,0.0f);
                //cameraChanged();
                break;
            case 'z':
                view->move(0.0f,1.0f,0.0f);
                //cameraChanged();
                break;
            case 'x':
                view->move(0.0f,-1.0f,0.0f);
                //cameraChanged();
                break;
            case '+':
                mass+=100.0f;
                break;
            case '-':
                mass-=100.0f;
                break;
            case '[':
                //sizeScale+=0.5f;
                {
                unsigned int res=systems["water"]->getSettings()->GetSettingAs<unsigned int>("color_field_res","32");
                res=res<<1;
                systems["water"]->getSettings()->SetSetting("color_field_res",res);
                break;
                }
            case ']':
                {
                //sizeScale-=0.5f;
                unsigned int res=systems["water"]->getSettings()->GetSettingAs<unsigned int>("color_field_res","32");
                res=res>>1;
                systems["water"]->getSettings()->SetSetting("color_field_res",res);
                break;
                }
            default:
                return;
        }
        glutPostRedisplay();
    }
void TestApplication::readParamFile(istream& is, string path)
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
        //Fixme::This is hacky. I need to determine an efficient way to do simulation scaling
        //for rigid bodies to work well with sph.
        if(sysSettings[i]->GetSettingAs<string>("system")!="sph")
        {
	    if(!sysSettings[i]->Exists("smoothing_distance") ||  !sysSettings[i]->Exists("simulation_scale"))
            {
		    sysSettings[i]->SetSetting("smoothing_distance",systems["water"]->getSettings()->GetSettingAs<float>("smoothing_distance"));
		    sysSettings[i]->SetSetting("simulation_scale",systems["water"]->getSettings()->GetSettingAs<float>("simulation_scale"));
	    }
        }
        systems[names[i]]=RTPS::generateSystemInstance(sysSettings[i],cli);
        systemRenderType[names[i]] = "Points";
        dout<<"names[i] \'"<<names[i]<<"\'"<<endl;

    }
    gridMin = systems["water"]->getSettings()->GetSettingAs<float4>("domain_min");
    gridMax = systems["water"]->getSettings()->GetSettingAs<float4>("domain_max");
    for(map<std::string,System*>::iterator i = systems.begin(); i!=systems.end(); i++)
    {
        for(map<std::string,System*>::iterator j = systems.begin(); j!=systems.end(); j++)
        {
            if(i==j)
                continue;
            //FIXME: More hacking. Don't add flocks to interaction systems yet!
            //The framework isn't defined for systems interacting with flocks yet.
            if(i->second->getSettings()->GetSettingAs<string>("system")=="flock"||
                    j->second->getSettings()->GetSettingAs<string>("system")=="flock")
                continue;
            i->second->addInteractionSystem(j->second);
        }
    }
}

    int TestApplication::writeMovieFrame(const char* filename, const char* dir)
    {
	GLubyte* image = new GLubyte[width()*height()*3];
	dout<<"width = "<<width()<<" height = "<<height()<<endl;
        stringstream s;
        s<<dir<<filename;
        s.fill('0');
        s.width(8);
        s<<right<<frameCounter;
        s<<".png";
        //sprintf(filename,"%s%s_%08d.png",render_dir,filename,frameCounter);
        glReadPixels(0, 0, width(), height(), GL_RGB, GL_UNSIGNED_BYTE, image);
        if (!stbi_write_png(s.str().c_str() , width(), height(),3,(void*)image,0))
        {
            delete[] image;
            cerr<<"failed to write image "<<filename<<endl;
            return -1;
        }
        delete[] image;
        return 0;
    }
ParticleShape* TestApplication::createParticleShape(const std::string& system, Mesh* mesh, float scale)
{
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    float* pos = new float[mesh->vboSize*3];
    glGetBufferSubData(GL_ARRAY_BUFFER,0,mesh->vboSize*3*sizeof(float),pos);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    float3 minCoord(FLT_MAX,FLT_MAX,FLT_MAX);
    float3 maxCoord(-FLT_MAX,-FLT_MAX,-FLT_MAX);
    for(int i = 0; i < mesh->vboSize; i++)
	{
            float x=pos[(i*3)],y=pos[(i*3)+1],z=pos[(i*3)+2];
            if(x<minCoord.x)
                minCoord.x=x;
            if(x>maxCoord.x)
                maxCoord.x=x;
            if(y<minCoord.y)
                minCoord.y=y;
            if(y>maxCoord.y)
                maxCoord.y=y;
            if(z<minCoord.z)
                minCoord.z=z;
            if(z>maxCoord.z)
                maxCoord.z=z;
    }
    minCoord.print("minCoord");
    maxCoord.print("maxCoord");
    float space = systems[system]->getSpacing();
    float halfspace = (space/2.0f);
    minCoord = minCoord-float3(halfspace,halfspace,halfspace);
    maxCoord = maxCoord+float3(halfspace,halfspace,halfspace);
	delete[] pos;
	pos =0;
    ParticleShape* shape = new ParticleShape(minCoord,maxCoord,space,scale);


    shape->voxelizeMesh(mesh->vbo,mesh->ibo,mesh->iboSize);
    //RenderUtils::write3DTextureToDisc(shape->getVoxelTexture(),shape->getVoxelResolution(),s.str().c_str());
    //shape->voxelizeSurface(me->vbo,me->ibo,me->iboSize);
    //s<<"surface";
    //RenderUtils::write3DTextureToDisc(shape->getSurfaceTexture(),shape->getVoxelResolution(),s.str().c_str());

    /*dout<<"mesh name = "<<s.str()<<endl;
    dout<<"maxCoord dim = "<<shape->getMaxDim()<<endl;
    dout<<"minCoord dim = "<<shape->getMinDim()<<endl;
    dout<<"Trans = "<<trans<<endl;
    dout<<"minCoord ("<<shape->getMin().x<<","<<shape->getMin().y<<","<<shape->getMin().z<<")"<<endl;
    dout<<"voxel res = "<<shape->getVoxelResolution()<<endl;
    dout<<"spacing = "<<space<<endl;*/

    return shape;

}
    void TestApplication::DestroyCallback()
    {

    }

void TestApplication::cameraChanged()
{
  //update the view matricies for all shaders.
    for(std::map<std::string,Shader>::iterator i = lib->shaders.begin(); i!=lib->shaders.end(); i++)
    {
        glUseProgram(i->second.getProgram());
        GLint location = glGetUniformLocation(i->second.getProgram(),"viewMatrix");
        if(location!=-1)
            glUniformMatrix4fv(location,1,GL_FALSE,view->getViewMatrix().m);
        location = glGetUniformLocation(i->second.getProgram(),"inverseViewMatrix");
        if(location!=-1)
            glUniformMatrix4fv(location,1,GL_FALSE,view->getInverseViewMatrix().m);
        location = glGetUniformLocation(i->second.getProgram(),"normalMatrix");
        if(location!=-1)
            glUniformMatrix4fv(location,1,GL_TRUE,view->getInverseViewMatrix().m);

    }
    glUseProgram(0);
}

    void TestApplication::TimerCallback(int ms)
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
    void TestApplication::ResetSimulations()
    {

    }
    void TestApplication::drawString(const char *str, int x, int y, float color[4], void *font)
    {
        glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
        glDisable(GL_LIGHTING);     // need to disable lighting for proper text color

        glColor4fv(color);          // set text color
        glRasterPos2i(x, y);        // place text position

        // loop all characters in the string
        while (*str)
        {
            glutBitmapCharacter(font, *str);
            ++str;
        }

        //glEnable(GL_LIGHTING);
        glPopAttrib();
    }

void TestApplication::renderSkyBox()
 {
     glPushAttrib(GL_ENABLE_BIT|GL_TEXTURE_BIT);
     glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
     glDisable(GL_DEPTH_TEST);
     glDisable(GL_TEXTURE_2D);
     glEnable(GL_TEXTURE_GEN_S);
     glEnable(GL_TEXTURE_GEN_T);
     glEnable(GL_TEXTURE_GEN_R);
     glEnable(GL_TEXTURE_CUBE_MAP);

     glUseProgram(lib->shaders["skybox"].getProgram());
     glActiveTexture(GL_TEXTURE0);
     glBindTexture(GL_TEXTURE_CUBE_MAP,environTex);

     // draw the skybox

     glEnableVertexAttribArray(0);
     glEnableVertexAttribArray(1);
     glBindBuffer(GL_ARRAY_BUFFER,skyboxVBO);
     glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,0);
     glBindBuffer(GL_ARRAY_BUFFER,skyboxTexVBO);
     glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,0,0);

     glDrawArrays(GL_QUADS, 0, 24);
     glPopAttrib();
     glPopClientAttrib();

     glDisableVertexAttribArray(0);
     glDisableVertexAttribArray(1);

     //glDisable(GL_TEXTURE_CUBE_MAP);

     //glDisable(GL_TEXTURE_GEN_R);
     glUseProgram(0);

 }


#if 0

    void TestApplication::displayShape(ParticleShape* shape,float3 translation,float spacing)
    {
        GLuint tex3d=shape->getVoxelTexture();
        float minDim=shape->getMinDim();
        float maxDim=shape->getMaxDim();
        unsigned int voxelResolution=shape->getVoxelResolution();
        vector<float> vec;
        glBindTexture(GL_TEXTURE_3D_EXT,tex3d);
        GLubyte* image = new GLubyte[voxelResolution*voxelResolution*voxelResolution*4];
        glGetTexImage(GL_TEXTURE_3D_EXT,0,GL_RGBA,GL_UNSIGNED_BYTE,image);
        float scale = (maxDim-minDim)*1.5;
        float16 modelview;
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        //glRotatef(rotation.x, 1.0, 0.0, 0.0);
        //glTranslatef(translation.x, translation.y, translation.z);
        glRotatef(-180, 1.0, 0.0, 0.0);
        glRotatef(-90, 0.0, 0.0, 1.0);
        glGetFloatv(GL_MODELVIEW_MATRIX,modelview.m);
        glPopMatrix();
        modelview.print("modelview");
        modelview.transpose();


        for(unsigned int k = 0; k<voxelResolution; k++)
        {
            for(unsigned int j=0; j<voxelResolution; j++)
            {
                for(unsigned int i=0;i<voxelResolution;i++)
                {
                    //Check the red channel. If it is non zero then we need a particle here.
                    if(image[(i*4)+(j*voxelResolution*4)+(k*voxelResolution*voxelResolution*4)]>0)
                    {
                        float4 pos;
                        pos.x = ((j/(float)(voxelResolution-1))-0.5f)*scale;
                        pos.y = ((i/(float)(voxelResolution-1))-0.5f)*scale;
                        pos.z = ((k/(float)(voxelResolution-1))-0.5f)*scale;
                        pos.w = 1.0f;
                        pos = modelview*pos;
                        vec.push_back(pos.x);
                        vec.push_back(pos.y);
                        vec.push_back(pos.z);
                    }
                }
            }
        }
        glBindTexture(GL_TEXTURE_3D_EXT,0);
        delete[] image;
        glEnable(GL_POINT_SMOOTH);
        glPointSize(spacing/2);


        glEnable(GL_POINT_SPRITE);
        glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
        GLuint program = lib->shaders["sphereLightShader"].getProgram();

        glUseProgram(program);
        glUniform1f( glGetUniformLocation(program, "pointScale"), ((float)windowWidth) / tanf(65.f * (0.5f * 3.1415926535f/180.0f)));

        float nf[2];
        glGetFloatv(GL_DEPTH_RANGE,nf);
        glUniform1f( glGetUniformLocation(program, "pointRadius"),spacing);
        glUniform1f( glGetUniformLocation(program, "near"),nf[0]);
        glUniform1f( glGetUniformLocation(program, "far"), nf[1]);


        glPushMatrix();
        glTranslatef(translation.x, translation.y, translation.z);
        GLuint shapeVBO = createVBO(&vec[0], vec.size()*sizeof(float), GL_ARRAY_BUFFER, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER,shapeVBO);
        glEnableClientState( GL_VERTEX_ARRAY );
        glVertexPointer(3, GL_FLOAT, 0, 0);
        glDrawArrays(GL_POINTS,0,vec.size()/3);
        glPopMatrix();

        glDisableClientState( GL_VERTEX_ARRAY );
        glUseProgram(0);
        glDeleteBuffers(1,&shapeVBO);
        glBindBuffer(GL_ARRAY_BUFFER,0);

        glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
        glDisable(GL_POINT_SPRITE);
    }
#endif



    void TestApplication::display(bool blend)
    {
        if(blend)
        {
            glEnable(GL_CULL_FACE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        }
        for(map<std::string,Mesh*>::iterator i = meshes.begin(); i!=meshes.end(); i++)
        {
            if((!blend&&i->second->material.opacity==1.0f) || (blend &&i->second->material.opacity<1.0f))
	    {
                meshRenderer->render(i->second,light);
	    }
        }
        if(blend)
        {
            glDisable(GL_CULL_FACE);
            glDisable(GL_BLEND);
        }
    }
    void TestApplication::loadMeshScene(const string& filename)
    {
	dynamicMeshScene->loadScene(filename);
	dynamicMeshScene->loadMeshes(dynamicMeshes,dynamicMeshScene->getScene()->mRootNode);
	for(map<std::string,Mesh*>::iterator i = dynamicMeshes.begin(); i!=dynamicMeshes.end(); i++)
	{
	  ParticleShape* shape = createParticleShape("rb1",i->second);
	  pShapes[i->first]=shape;
	}
	currentMesh = dynamicMeshes.begin()->first;
    }
    void TestApplication::loadScene(const string& filename)
    {
	scene->loadScene(filename);
	scene->loadMeshes(meshes,scene->getScene()->mRootNode);
	for(map<std::string,Mesh*>::iterator i = meshes.begin(); i!=meshes.end(); i++)
	{
		ParticleShape* shape = createParticleShape("rb1",i->second);
		//i->second->modelMat.m[12]=-5.;
		//i->second->modelMat.m[13]=-5.;
		//i->second->modelMat.m[14]=5.;
		float trans = (shape->getMaxDim()+shape->getMinDim())/2.0f;
		//float16 mat=i->second->modelMat;
		//FIXME: rotation should not be needed here. This is hackish
		float16 mat;
		mat.loadIdentity();
		mat[5]=-1.0f;
		mat[10]=-1.0f;
		//mat.transpose();
		float16 mat2;
		mat2.loadIdentity();
		mat2[0]=0.0f;
		mat2[1]=-1.f;
		mat2[4]=1.f;
		mat2[5]=0.0f;
		mat=mat2*mat;

		//memcpy(&mat,&i->second->modelMat,sizeof(float16));
		//mat.print("mat before");
		mat[3]+=trans;
		mat[7]+=trans;
		mat[11]+=trans;
		//mat=mat2*mat;
		//mat = mat*view->getViewMatrix();
		//mat.print("mat after");
		systems["rb1"]->addParticleShape(shape->getVoxelTexture(),shape->getMinDim(),shape->getMaxDim(),mat,shape->getVoxelResolution(),float4(0.0f,0.0f,0.0f,0.0f),float4(1.0f,0.5f,0.0f,1.0f),0.0f);
		pShapes[i->first]=shape;
	}
    }
void TestApplication::addRigidBody(const std::string& system, const std::string& mesh, float4 pos, float4 vel, float mass)
{
    currentMesh=mesh;
    ParticleShape* shape = pShapes[mesh];
    //float trans = (shape->getMaxDim()+shape->getMinDim())/2.0f;
    //float16 mat=i->second->modelMat;
    //FIXME: rotation should not be needed here. This is hackish
    float16 mat;
    mat.loadIdentity();
    mat[5]=-1.0f;
    mat[10]=-1.0f;
    //mat.transpose();
    float16 mat2;
    mat2.loadIdentity();
    mat2[0]=0.0f;
    mat2[1]=-1.f;
    mat2[4]=1.f;
    mat2[5]=0.0f;
    mat=mat2*mat;

    //memcpy(&mat,&i->second->modelMat,sizeof(float16));
    //mat.print("mat before");
    mat[3]+=pos.x;
    mat[7]+=pos.y;
    mat[11]+=pos.z;
    systems[system]->addParticleShape(shape->getVoxelTexture(),shape->getMinDim(),shape->getMaxDim(),mat,shape->getVoxelResolution(),vel,float4(1.0f,0.5f,0.0f,1.0f),mass);

}
void TestApplication::createSceneTextures()
{
    int num = 2;
    if(stereoscopic)
        num=4;
    if(sceneTex[0])
        glDeleteTextures(num,sceneTex);
    glPushAttrib(GL_ENABLE_BIT|GL_TEXTURE_BIT);
    glEnable(GL_TEXTURE_2D);
    glGenTextures(num, sceneTex);
    glBindTexture(GL_TEXTURE_2D, sceneTex[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT32,width(),height(),0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);
    glBindTexture(GL_TEXTURE_2D, sceneTex[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width(),height(),0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
    if(stereoscopic)
    {
        glBindTexture(GL_TEXTURE_2D, sceneTex[3]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT32,width(),height(),0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);
        glBindTexture(GL_TEXTURE_2D, sceneTex[2]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width(),height(),0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
    }
    glDisable(GL_TEXTURE_2D);
    glPopAttrib();

}


    void TestApplication::setWindowHeight(GLuint windowHeight) {
        this->windowHeight = windowHeight;
    }

    void TestApplication::setWindowWidth(GLuint windowWidth) {
        this->windowWidth = windowWidth;
    }
};
