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
#include <QGLWidget>
#include <QElapsedTimer>
#include <QString>
#include <QTimer>
#include <QMouseEvent>
#include <QKeyEvent>

#include "ParamParser.h"
#include "../rtpslib/system/ParticleShape.h"
#include "../rtpslib/RTPS.h"
#include "../rtpslib/render/ParticleEffect.h"
#include "../rtpslib/render/SSEffect.h"
#include "../rtpslib/render/MeshEffect.h"
#include "../rtpslib/system/ParticleRigidBody.h"
#include "../rtpslib/system/SPH.h"
#include "../rtpslib/system/FLOCK.h"

#include <math.h>
#include <sstream>
#include <float.h>

#include "glwidget.h"
#include "aiwrapper.h"

#include "../rtpslib/render/util/stb_image_write.h"

using namespace std;
namespace rtps
{
 GLWidget::GLWidget(QGLContext* ctx,std::string bPath,QWidget *parent)
     : QGLWidget(ctx,parent)
 {
    binaryPath=bPath;
         //mass=100.0f;
    mass=0.01f;
    sizeScale=1.0f;
    //string scenefile = path+"/demo_scene.obj";

    renderVelocity=false;
    paused=false;
    scene_list=0;
    scene=new AIWrapper();
    dynamicMeshScene=new AIWrapper();
    renderMovie=false;
    frameCounter=0;
    cli=NULL;
    view=NULL;
    light=NULL;
    lib=NULL;
    elapsedTimer = new QElapsedTimer();

    //This will force updates every 33 seconds. The timer will cause the
    //simulations to perform their updates.
    QTimer* timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(30);
 }

 GLWidget::~GLWidget()
 {
    makeCurrent();
    for(map<QString,System*>::iterator i = systems.begin(); i!=systems.end(); i++)
    {
        delete i->second;
    }
    for(map<QString,ParticleEffect*>::iterator i = effects.begin(); i!=effects.end(); i++)
    {
        delete i->second;
    }
    for(map<QString,ParticleShape*>::iterator i = pShapes.begin(); i!=pShapes.end(); i++)
    {
        delete i->second;
    }
    for(map<QString,Mesh*>::iterator i = meshes.begin(); i!=meshes.end(); i++)
    {
        delete i->second;
    }
    delete meshRenderer;
    delete dynamicMeshScene;
    delete scene;
    delete cli;
    delete lib;
    delete view;
    delete light;
    delete elapsedTimer;
 }

 void GLWidget::initializeGL()
 {

     //needs to call make current before glew init. Seems to
     //not have a valid context otherwise.
     makeCurrent();
     glewInit();
     //TODO: Add stereo camera
     if(!view)
     {
        //view = new Camera(float3(5.0f,5.0f,15.0f),65.0,0.3,100.0,width(),height());
         view = new Camera(float3(5.0f,5.0f,15.0f),65.0,0.3,100.0,width(),height());
        //view = new Camera(float3(0.0f,0.0f,0.0f),65.0f,0.3f,100.0f,width(),height());
        //view = new Camera(float3(-105.0f,-105.0f,-105.0f),65.0f,0.3f,1000.0f,width(),height());
        //the models are all in different coordinate systems...
        //view->rotate(-90.f,0.0f);
        view->setMoveSpeed(2.f);
        view->setRotateSpeed(2.f);
     }

     //DEBUGGING===========
     //const float16& myProjectionMatrix = view->getProjectionMatrix();
     //const float16& myViewMatrix = view->getViewMatrix();
     //float16 projectionMatrix;
     //float16 viewMatrix;

     // projection
     //glMatrixMode(GL_PROJECTION);
     //glLoadIdentity();
     //gluPerspective(60.0, (GLfloat)window_width / (GLfloat) window_height, 0.1, 100.0);
     //gluPerspective(65.0, (double)width() / (double)height(), 0.3, 100.0);
     //gluPerspective(90.0, (GLfloat)window_width / (GLfloat) window_height, 0.1, 10000.0); //for lorentz

     // set view matrix
     //glClearColor(.9f, .9f, .9f, 1.0f);
     //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     //glMatrixMode(GL_MODELVIEW);
     //glLoadIdentity();
     //glRotatef(-90, 1.0, 0.0, 0.0);

     //glRotatef(0.0, 1.0, 0.0, 0.0);
     //glRotatef(0.0, 0.0, 0.0, 1.0); //we switched around the axis so make this rotate_z
     //glTranslatef(-5., 5., -5.);

     //glGetFloatv(GL_PROJECTION_MATRIX,projectionMatrix.m);
     //glGetFloatv(GL_MODELVIEW_MATRIX,viewMatrix.m);

     //if(!(myProjectionMatrix==projectionMatrix))
     //{
     //    dout<<"I'm not correctly caclulating my projection matrix."<<endl;
     //}
     //myProjectionMatrix.print("myProjectionMatrix");
     //projectionMatrix.print("projectionMatrix");
     //if(!(viewMatrix==myViewMatrix))
     //{
     //    dout<<"I'm not correctly calculating my view Matrix."<<endl;
     //}
     //glGetFloatv(GL_MODELVIEW_MATRIX,viewMatrix.m);
     //myViewMatrix.print("viewMatrix");
     //viewMatrix.print("viewMatrix");

     //DEBUGGING===========

     if(!cli)
        cli = new CL();

     if(!light)
     {
         light = new Light();
        light->diffuse.x=1.0;light->diffuse.y=1.0;light->diffuse.z=1.0;
        light->ambient.x=1.0;light->ambient.y=1.0;light->ambient.z=1.0;
        light->specular.x=1.0;light->specular.y=1.0;light->specular.z=1.0;
        //light->pos.x=-0.5f; light->pos.y=1.5f; light->pos.z=5.0f;
        light->pos.x=5.0f; light->pos.y=5.0f; light->pos.z=5.0f;

    }
    environTex = RenderUtils::loadCubemapTexture(binaryPath+"/cubemaps/");


    glViewport(0, 0, width(), height());
    if(!lib){
        lib = new ShaderLibrary();
        lib->initializeShaders(binaryPath+"/shaders");
        effects["Points"]=new ParticleEffect(lib,width(),height(),.75f,false);
        effects["Screen Space"]=new SSEffect(lib,GAUSSIAN_BLUR,width(),height(),.75f,true);
        effects["Mesh Renderer"]= new MeshEffect(lib,width(),height(),.75f,false);

        //FIXME: Need to find an elegant solution to handling mesh effects
        meshRenderer= new MeshEffect(lib,width(),height(),20.0f,false);
    }

    glClearColor(0.9f,0.9f,0.9f,1.0f);
    //set the projection and view matricies for all shaders.
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
        glUseProgram(0);
    }
    GLuint program = lib->shaders["sphereShader"].getProgram();
    glUseProgram(program);
    glUniform1f( glGetUniformLocation(program, "pointScale"), ((float)width()) / tanf(view->getFOV()* (0.5f * PIOVER180)));
    program = lib->shaders["skybox"].getProgram();
    glUseProgram(program);
    glUniform1i(glGetUniformLocation(program, "skyboxCubeSampler"), 0);
    glUseProgram(0);
    elapsedTimer->start();
 }

 void GLWidget::renderSkyBox()
 {
     glDisable(GL_TEXTURE_2D);
     glEnable(GL_TEXTURE_GEN_S);
     glEnable(GL_TEXTURE_GEN_T);
     glEnable(GL_TEXTURE_GEN_R);
     glEnable(GL_TEXTURE_CUBE_MAP);

     glUseProgram(lib->shaders["skybox"].getProgram());
     glActiveTexture(GL_TEXTURE0);
     glBindTexture(GL_TEXTURE_CUBE_MAP,environTex);

     // draw the skybox
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

     //glVertexPointer(3, GL_FLOAT, 0, skyBox);
     //glTexCoordPointer(3, GL_FLOAT, 0, skyBoxTex);
     glEnableVertexAttribArray(0);
     glEnableVertexAttribArray(1);
     glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,skyBox);
     glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,0,skyBoxTex);

     glDrawArrays(GL_QUADS, 0, 6);

     glDisableVertexAttribArray(0);
     glDisableVertexAttribArray(1);

     glDisable(GL_TEXTURE_CUBE_MAP);

     glDisable(GL_TEXTURE_GEN_R);
     glUseProgram(0);

 }

 void GLWidget::paintGL()
 {
        float time = elapsedTimer->restart()/1000.f;
        //send new view matrix to the shaders if the camera was actually updated
        if(view->tick(time))
        {
            cameraChanged();
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_MULTISAMPLE_ARB);
#if 1
        //renderSkyBox();
        //display static opaque objects
        display(false);
#if 1
#if 1
        if(systems.find(QString("rb1"))!=systems.end())
        {
            //for debugging only!!
            ParticleRigidBody* rbsys = (ParticleRigidBody*)systems[QString("rb1")];
            effects["Points"]->render(rbsys->getStaticVBO(),rbsys->getColVBO(),rbsys->getStaticNum(),light,NULL,0.05f);
        }
#endif
        for(map<QString,System*>::iterator i = systems.begin(); i!=systems.end(); i++)
        {
            if(renderVelocity)
            {
                effects[systemRenderType[i->first]]->renderVector(i->second->getPosVBO(),i->second->getVelocityVBO(),i->second->getNum());
            }
            effects[systemRenderType[i->first]]->render(i->second->getPosVBO(),i->second->getColVBO(),i->second->getNum(),light,NULL,i->second->getSpacing());
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
        glDisable(GL_MULTISAMPLE_ARB);
        if(renderMovie)
        {
            writeMovieFrame("image","./frames/");
            frameCounter++;
        }
 }

void GLWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
    for(map<QString,ParticleEffect*>::iterator i = effects.begin(); i!=effects.end(); i++)
    {
        i->second->setWindowDimensions(width,height);
    }
    view->setWidth(width);
    view->setHeight(height);
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
}

 void GLWidget::mousePressEvent(QMouseEvent *event)
 {
        mouseButtons |= event->button();
        mousePos.x = event->x();
        mousePos.y = event->y();
        //updateGL();
 }
 void GLWidget::mouseReleaseEvent(QMouseEvent *event)
 {
        mouseButtons &= !event->button();
        mousePos.x = event->x();
        mousePos.y = event->y();
        //updateGL();
 }

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    float dx, dy;
    dx = event->x() - mousePos.x;
    dy = event->y() - mousePos.y;

    if (mouseButtons & Qt::RightButton)
    {
        //view->rotate(dx,dy);
        view->rotate(dy,dx);
        //view->rotate(dx,0.0);
        //glRotatef(dx*.2f,1.0f,0.0f,0.0f);
        //glRotatef(dy*.2f,0.0f,0.0f,1.0f);
        //cameraChanged();
    }

    mousePos.x = event->x();
    mousePos.y = event->y();
    updateGL();
}

void GLWidget::cameraChanged()
{

    //view->getViewMatrix().print("myViewMatrix");
    //float16 viewMatrix;
    //glGetFloatv(GL_MODELVIEW_MATRIX,viewMatrix.m);
    //viewMatrix.print("viewMatrix");
    //update the view matricies for all shaders.
    for(std::map<std::string,Shader>::iterator i = lib->shaders.begin(); i!=lib->shaders.end(); i++)
    {
        glUseProgram(i->second.getProgram());
        GLint location = glGetUniformLocation(i->second.getProgram(),"viewMatrix");
        if(location!=-1)
            glUniformMatrix4fv(location,1,GL_FALSE,view->getViewMatrix().m);
            //glUniformMatrix4fv(location,1,GL_FALSE,view->getInverseViewMatrix().m);
        location = glGetUniformLocation(i->second.getProgram(),"inverseViewMatrix");
        if(location!=-1)
            glUniformMatrix4fv(location,1,GL_FALSE,view->getInverseViewMatrix().m);
        location = glGetUniformLocation(i->second.getProgram(),"normalMatrix");
        if(location!=-1)
            //glUniformMatrix4fv(location,1,GL_FALSE,view->getViewMatrix().m);
            glUniformMatrix4fv(location,1,GL_TRUE,view->getInverseViewMatrix().m);

    }
}

void GLWidget::keyPressEvent(QKeyEvent *event)
{
        //FIXME: Need to update key bindings.
        if(event->text().isEmpty())
            return;
        char key=event->text().at(0).toAscii();
        unsigned int nn=0;
        switch (key)
        {
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
                float4 col1 = float4(0.05f, 0.1f, .2f, 0.1f);
                float4 center = float4(gridMax.x-2.0f, gridMax.y-2.0f,gridMax.z-1.5f,1.0f);
                float4 velocity(-1.25f, -1.25f, -3.0f, 0);
                float radius= 2.0f;
                //sph sets spacing and multiplies by radius value
                systems["water"]->addHose(1000, center, velocity,radius, col1);
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
updateGL();
}
void GLWidget::readParamFile(std::istream& is)
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
                sysSettings[i]->SetSetting("smoothing_distance",systems["water"]->getSettings()->GetSettingAs<float>("smoothing_distance"));
                sysSettings[i]->SetSetting("simulation_scale",systems["water"]->getSettings()->GetSettingAs<float>("simulation_scale"));
            }
            systems[QString(names[i].c_str())]=RTPS::generateSystemInstance(sysSettings[i],cli);
            systemRenderType[QString(names[i].c_str())] = "Points";
            dout<<"names[i] \'"<<names[i]<<"\'"<<endl;

        }
        gridMin = systems["water"]->getSettings()->GetSettingAs<float4>("domain_min");
        gridMax = systems["water"]->getSettings()->GetSettingAs<float4>("domain_max");
        for(map<QString,System*>::iterator i = systems.begin(); i!=systems.end(); i++)
        {
            for(map<QString,System*>::iterator j = systems.begin(); j!=systems.end(); j++)
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
        emit systemMapChanged(names);
}
int GLWidget::writeMovieFrame(const char* filename, const char* dir)
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
ParticleShape* GLWidget::createParticleShape(const QString& system, Mesh* mesh)
{
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    float* pos = new float[mesh->vboSize*3];
    glGetBufferSubData(GL_ARRAY_BUFFER,0,mesh->vboSize*3*sizeof(float),pos);
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
	delete[] pos;
	pos =0;
    float space = systems[system]->getSpacing();
    ParticleShape* shape = new ParticleShape(minCoord,maxCoord,space);


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
    void GLWidget::display(bool blend)
    {
        glEnable(GL_NORMALIZE);
        glEnable(GL_CULL_FACE);
        //glDisable(GL_LIGHTING);
        if(blend)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        }
        for(map<QString,Mesh*>::iterator i = meshes.begin(); i!=meshes.end(); i++)
        {
            if((!blend&&i->second->material.opacity==1.0f) || (blend &&i->second->material.opacity<1.0f))
                meshRenderer->render(i->second,light);
        }
        if(blend)
        {
            glDisable(GL_BLEND);
        }
        glDisable(GL_NORMALIZE);
        glDisable(GL_CULL_FACE);
    }

void GLWidget::setParameterValue(const QString& system, const QString& parameter, const QString& value)
{
    systems[system]->getSettings()->SetSetting(std::string(parameter.toAscii().data()),std::string(value.toAscii().data()));
}
void GLWidget::loadScene(const QString& filename)
{
     scene->loadScene(filename);
     scene->loadMeshes(meshes,scene->getScene()->mRootNode);
     for(map<QString,Mesh*>::iterator i = meshes.begin(); i!=meshes.end(); i++)
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
            mat.print("mat before");
            mat2.print("mat2");
            mat=mat2*mat;
            mat.print("mat after");

            //memcpy(&mat,&i->second->modelMat,sizeof(float16));
            //mat.print("mat before");
            mat[3]+=trans;
            mat[7]+=trans;
            mat[11]+=trans;
            //mat=mat2*mat;
            //mat = mat*view->getViewMatrix();
            //mat.print("mat after");
          systems["rb1"]->addParticleShape(shape->getVoxelTexture(),shape->getMinDim(),shape->getMaxDim(),mat,shape->getVoxelResolution(),float4(0.0f,0.0f,0.0f,0.0f),float4(0.0f,0.0f,0.0f,1.0f),0.0f);
          pShapes[i->first]=shape;
     }
}
void GLWidget::loadMeshScene(const QString& filename)
{
     dynamicMeshScene->loadScene(filename);
     dynamicMeshScene->loadMeshes(dynamicMeshes,dynamicMeshScene->getScene()->mRootNode);
     for(map<QString,Mesh*>::iterator i = dynamicMeshes.begin(); i!=dynamicMeshes.end(); i++)
     {
          ParticleShape* shape = createParticleShape("rb1",i->second);
          pShapes[i->first]=shape;
     }
}


void GLWidget::loadParameterFile(const QString& filename)
{
	ifstream is(filename.toAscii().data(),ifstream::in);
    readParamFile(is);
}
void GLWidget::ResetSimulations()
{

}
void GLWidget::update()
{
        if(!paused)
        {
            glFinish();
            for(map<QString,System*>::iterator i = systems.begin(); i!=systems.end(); i++)
            {
                i->second->acquireGLBuffers();
                i->second->update();
                i->second->interact();
            }

            for(map<QString,System*>::iterator i = systems.begin(); i!=systems.end(); i++)
            {
                i->second->integrate();
                i->second->postProcess();
                i->second->releaseGLBuffers();
            }
        }
	updateGL();
}
void GLWidget::changeRenderer(const QString& system, const QString& renderer)
{
     //dout<<"system = "<<(const char*)system.toAscii().data()<<"renderer = "<<(const char*)renderer.toAscii().data()<<endl;
     systemRenderType[system]=renderer;
     //FIXME: temporary Debugging hack!
     effects[renderer]->writeBuffersToDisk();
}
}
