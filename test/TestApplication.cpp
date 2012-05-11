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
#include "../rtpslib/render/MeshEffect.h"
#include <../rtpslib/system/ParticleRigidBody.h>
#include <../rtpslib/system/SPH.h>
#include <../rtpslib/system/FLOCK.h>

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
    TestApplication::TestApplication(istream& is, string path)
    {
        glewInit();
        GLboolean bGLEW = glewIsSupported("GL_VERSION_2_0 GL_ARB_pixel_buffer_object");
        windowHeight=800;
        windowWidth=600;
        cli = new CL();

        renderType="default";
        readParamFile(is,path);
        initGL();
        //Fixme: This is a bad way to make sure the directory is correct.
        RenderSettings rs;
        //rs.blending=false;
        rs.blending=false;
        float nf[2];
        glGetFloatv(GL_DEPTH_RANGE,nf);
        rs.m_near = nf[0];
        rs.m_far = nf[1];
        //dout<<"near = "<<rs.near<<endl;
        //dout<<"far = "<<rs.far<<endl;
        //dout<<"spacing = "<<systems["water"]->getSpacing()<<endl;
        rs.particleRadius = systems["water"]->getSpacing()*20.f;
        rs.windowWidth=windowWidth;
        rs.windowHeight=windowHeight;
        lib = new ShaderLibrary();
		string shaderpath=path+"/shaders";
        lib->initializeShaders(shaderpath);
        effects["default"]=new ParticleEffect(rs,*lib);
        //effects["sprite"]=new ParticleEffect();
        rs.blending=true;
        rs.particleRadius =systems["water"]->getSpacing()*.6f;
        effects["ssfr"]=new SSEffect(rs, *lib);
        meshRenderer=new MeshEffect(rs, *lib);
        translation.x = -5.00f;
        translation.y = -5.00f;//300.f;
        translation.z = 5.00f;
        rotation.x=0.0f;
        rotation.y=0.0f;
        light.diffuse.x=1.0;light.diffuse.y=1.0;light.diffuse.z=1.0;
        //light.ambient.x=0.3;light.ambient.y=0.3;light.ambient.z=0.3;
        light.ambient.x=1.0;light.ambient.y=1.0;light.ambient.z=1.0;
        light.specular.x=1.0;light.specular.y=1.0;light.specular.z=1.0;
        light.pos.x=-0.5f; light.pos.y=1.5f; light.pos.z=5.0f;
        //mass=100.0f;
        mass=0.01f;
        sizeScale=1.0f;
        string scenefile = path+"/demo_scene.obj";

        renderVelocity=false;
        paused=false;
        scene=NULL;
        scene_list=0;
        loadScene(scenefile);
        renderMovie=false;
        frameCounter=0;
        string meshesfile = path+"/demo_mesh_scene.obj";
        loadMeshScene(meshesfile);
        build_shapes(scene, scene->mRootNode);
        build_dynamic_shapes(dynamicMeshScene, dynamicMeshScene->mRootNode);
        environTex = RenderUtils::loadCubemapTexture(path+"/cubemaps/");
    }

    void TestApplication::setWindowHeight(GLuint windowHeight) {
        this->windowHeight = windowHeight;
    }

    void TestApplication::setWindowWidth(GLuint windowWidth) {
        this->windowWidth = windowWidth;
    }
    TestApplication::~TestApplication()
    {
        for(map<string,System*>::iterator i = systems.begin(); i!=systems.end(); i++)
        {
            delete i->second;
        }
        for(map<string,ParticleEffect*>::iterator i = effects.begin(); i!=effects.end(); i++)
        {
            delete i->second;
        }
        for(map<string,ParticleShape*>::iterator i = pShapes.begin(); i!=pShapes.end(); i++)
        {
            delete i->second;
        }
        for(map<string,Mesh*>::iterator i = meshs.begin(); i!=meshs.end(); i++)
        {
            delete i->second;
        }
        delete meshRenderer;
        delete cli;
        delete lib;
    }
    void TestApplication::KeyboardCallback(unsigned char key, int x, int y)
    {
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
                ParticleShape* shape=pShapes["dynamicShape0"];
                float trans = (shape->getMaxDim()+shape->getMinDim())/2.0f;
                trans +=7.0f;
                float16 modelview;
                glMatrixMode(GL_MODELVIEW);
                glPushMatrix();
                glLoadIdentity();
                //glRotatef(rotation.x, 1.0, 0.0, 0.0);
                glTranslatef(trans, trans, trans);
                glRotatef(-180, 1.0f, 0.0f, 0.0f);
                glRotatef(-90, 0.0f, 0.0f, 1.0f);
                //glTranslatef(translation.x, translation.z, translation.y);
                glGetFloatv(GL_MODELVIEW_MATRIX,modelview.m);
                glPopMatrix();
                modelview.print("modelview");
                modelview.transpose();

                systems["rb1"]->addParticleShape(shape->getVoxelTexture(),shape->getMinDim(),shape->getMaxDim(),modelview,shape->getVoxelResolution(),float4(0.0f,0.0f,0.0f,0.0f),float4(0.0f,0.0f,0.0f,1.0f),mass);

                /*    float4 col1 = float4(0.5, 0.9, 0.0, 1.);
                    float size = 1.0f;
                    size=size*sizeScale;
                    float4 mid = (gridMax-gridMin);
                    mid = mid/2.0f;
                    mid.w = 0.0f;

                    systems["rb1"]->addBall(1000, mid, size,false, col1,mass);*/
                    return;
                }
            case 'v':
                renderVelocity=!renderVelocity;
                return;
            case 'o':
                renderType="default";
                return;
            case 'c':
                renderType="ssfr";
                return;
            case 'C':
                return;
            case '2':
                light.pos.z -= 0.1f;
                break;
            case '6':
                light.pos.x += 0.1f;
                break;
            case '8':
                light.pos.z += 0.1f;
                break;
            case '4':
                light.pos.x -= 0.1f;
                break;
            case '3':
                light.pos.y += 0.1f;
                break;
            case '1':
                light.pos.y -= 0.1f;
                break;
            case 'w':
                translation.z -= 0.1f;
                break;
            case 'a':
                translation.x += 0.1f;
                break;
            case 's':
                translation.z += 0.1f;
                break;
            case 'd':
                translation.x -= 0.1f;
                break;
            case 'z':
                translation.y += 0.1f;
                break;
            case 'x':
                translation.y -= 0.1f;
                break;
            case '+':
                mass+=100.0f;//0.1f;
                break;
            case '-':
                mass-=100.0f;//0.1f;
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
    void TestApplication::RenderCallback()
    {

        glClearColor(.9f, .9f, .9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_MULTISAMPLE_ARB);
#if 1
        /*if (stereo_enabled)
        {
            render_stereo();
        }
        else
        {*/
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            gluPerspective(65.0, windowWidth/(double)windowHeight, 0.3, 1000.0);
                        // set view matrix


            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glPushMatrix();
            glRotatef(rotation.x, 1.0, 0.0, 0.0);
            glRotatef(rotation.y, 0.0, 0.0, 1.0); //we switched around the axis so make this rotate_z
            glTranslatef(translation.x, translation.z, translation.y);

            glDisable(GL_TEXTURE_2D);
            glEnable(GL_TEXTURE_GEN_S);
            glEnable(GL_TEXTURE_GEN_T);
            glEnable(GL_TEXTURE_GEN_R);
            glEnable(GL_TEXTURE_CUBE_MAP);

            glBindTexture(GL_TEXTURE_CUBE_MAP, environTex);
            // draw the skybox
            const GLfloat fSkyDist = 100.0;
            const GLfloat fTex = 1.0f;

            glBegin(GL_TRIANGLE_STRIP);

            //west
                glTexCoord3f(-fTex, -fTex , fTex);
                glVertex3f(-fSkyDist, -fSkyDist,  fSkyDist);
                glTexCoord3f(-fTex, fTex, fTex);
                glVertex3f(-fSkyDist, fSkyDist,  fSkyDist);
                glTexCoord3f(-fTex, -fTex, -fTex);
                glVertex3f(-fSkyDist, -fSkyDist, -fSkyDist);
                glTexCoord3f(-fTex, fTex,-fTex);
                glVertex3f(-fSkyDist, fSkyDist,  -fSkyDist);
            //north
                glTexCoord3f(fTex, -fTex,-fTex);
                glVertex3f(fSkyDist, -fSkyDist, -fSkyDist);
                glTexCoord3f(fTex, fTex,-fTex);
                glVertex3f(fSkyDist, fSkyDist, -fSkyDist);
            //east
                glTexCoord3f( fTex, -fTex, fTex);
                glVertex3f( fSkyDist, -fSkyDist,  fSkyDist);
                glTexCoord3f( fTex, fTex, fTex);
                glVertex3f( fSkyDist, fSkyDist,  fSkyDist);
            //south
                glTexCoord3f( -fTex, -fTex, fTex);
                glVertex3f( -fSkyDist, -fSkyDist,  fSkyDist);
                glTexCoord3f( -fTex, fTex, fTex);
                glVertex3f( -fSkyDist, fSkyDist,  fSkyDist);
            glEnd();
            glBegin(GL_QUADS);
            //up
                glTexCoord3f( -fTex, fTex, -fTex);
                glVertex3f( -fSkyDist, fSkyDist,  -fSkyDist);
                glTexCoord3f( -fTex, fTex, fTex);
                glVertex3f( -fSkyDist, fSkyDist,  fSkyDist);
                glTexCoord3f( fTex, fTex, fTex);
                glVertex3f( fSkyDist, fSkyDist,  fSkyDist);
                glTexCoord3f( fTex, fTex, -fTex);
                glVertex3f( fSkyDist, fSkyDist,  -fSkyDist);
            //down
                glTexCoord3f( fTex, -fTex, -fTex);
                glVertex3f( fSkyDist, -fSkyDist,  -fSkyDist);
                glTexCoord3f( fTex, -fTex, fTex);
                glVertex3f( fSkyDist, -fSkyDist,  fSkyDist);
                glTexCoord3f( -fTex, -fTex, fTex);
                glVertex3f( -fSkyDist, -fSkyDist,  fSkyDist);
                glTexCoord3f( -fTex, -fTex, -fTex);
                glVertex3f( -fSkyDist, -fSkyDist,  -fSkyDist);
            glEnd();

            glDisable(GL_TEXTURE_CUBE_MAP);

            glDisable(GL_TEXTURE_GEN_R);


            glPopMatrix();

            glRotatef(-90, 1.0, 0.0, 0.0);

            glRotatef(rotation.x, 1.0, 0.0, 0.0);
            glRotatef(rotation.y, 0.0, 0.0, 1.0); //we switched around the axis so make this rotate_z
            glTranslatef(translation.x, translation.z, translation.y);
            //Draw Origin - Found code at http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=248059

            float ORG[3] = {0,0,0};

            float XP[3] = {1,0,0}, XN[3] = {-1,0,0},
                  YP[3] = {0,1,0}, YN[3] = {0,-1,0},
                  ZP[3] = {0,0,1}, ZN[3] = {0,0,-1};
            glLineWidth (20.0);
            glBegin (GL_LINES);
            glColor3f (1,0,0); // X axis is red.
            glVertex3fv (ORG);
            glVertex3fv (XP );
            glColor3f (0,1,0); // Y axis is green.
            glVertex3fv (ORG);
            glVertex3fv (YP );
            glColor3f (0,0,1); // z axis is blue.
            glVertex3fv (ORG);
            glVertex3fv (ZP );
            glEnd();
            glLineWidth (1.0);


            //RenderUtils::renderBox(float4(light.pos.x-.5,light.pos.y-.5,light.pos.z-.5,1.0f),float4(light.pos.x+.5,light.pos.y+.5,light.pos.z+.5,1.0f),float4(.7,.2,.3,1.0f));
            ParticleRigidBody* rbsys = (ParticleRigidBody*)systems["rb1"];
            meshRenderer->renderInstanced(dynamicMeshs["dynamicShape0"],rbsys->getComPosVBO(),rbsys->getComRotationVBO(),rbsys->getNum(),light);
            if(systems.find("flock1")!=systems.end())
            {
                //dout<<"flock------------------"<<endl;
                FLOCK* flock = (FLOCK*)systems["flock1"];
                //effects[renderType]->render(flock->getPosVBO(),flock->getColVBO(),flock->getNum());
                meshRenderer->renderInstanced(dynamicMeshs["dynamicShape1"],flock->getPosVBO(),flock->getRotationVBO(),flock->getNum(),light);
            }
            display(false);

            //glDisable(GL_DEPTH_TEST);
            //RenderUtils::renderBox(gridMin,gridMax,float4(0.0f,1.0,0.0f,1.0f));
            //FIXME: Have a method to give renderType to each System. That way we can have different
            //Systems with the different effects.
            for(map<string,System*>::iterator i = systems.begin(); i!=systems.end(); i++)
            {
                if(renderVelocity)
                {
                    effects[renderType]->renderVector(i->second->getPosVBO(),i->second->getVelocityVBO(),i->second->getNum());
                }
                //effects[renderType]->render(i->second->getPosVBO(),i->second->getColVBO(),i->second->getNum());
                //FIXME:This is a horrible way of doing this!!
                //if(i->first=="rb1")
                //{
                //    ParticleRigidBody* prb=((ParticleRigidBody*)i->second);
                //    effects["default"]->render(prb->getStaticVBO(),prb->getColVBO(),prb->getStaticNum());
                //}
            }
            //FIXME: Super hacky! I should figure out betterways to determine how to render based on some settings.
            SPH* sph = (SPH*)systems["water"];
            glEnable(GL_DEPTH_TEST);
            //glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            //glBlendFunc(GL_ONE,GL_ONE);
            Mesh* mcMesh = sph->getMCMesh();
            glEnable(GL_BLEND);
            if(sph->getSettings()->GetSettingAs<bool>("use_color_field","0")&&mcMesh)
            {
		glEnable(GL_CULL_FACE);
                //meshRenderer->render(mcMesh,light);
                meshRenderer->renderFluid(mcMesh,environTex,0,light);
		glDisable(GL_CULL_FACE);
            }
            else
            {
                effects[renderType]->render(systems["water"]->getPosVBO(),systems["water"]->getColVBO(),systems["water"]->getNum());
            }
            display(true);
#else
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(65.0, windowWidth/(double)windowHeight, 0.3, 100.0);

        // set view matrix
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glRotatef(-90, 1.0, 0.0, 0.0);

        glRotatef(rotation.x, 1.0, 0.0, 0.0);
        glRotatef(rotation.y, 0.0, 0.0, 1.0); //we switched around the axis so make this rotate_z
        glTranslatef(translation.x, translation.z, translation.y);

        meshRenderer->render(dynamicMeshs["dynamicShape1"],light);
        glColor4f(0.1f,0.2f,0.4f,1.0f);
        displayShape(pShapes["dynamicShape1"],float3(5.0f,3.f,1.0f),systems["water"]->getSpacing());
        displayShape(pShapes["dynamicShape11"],float3(8.0f,3.f,1.0f),systems["water"]->getSpacing()/2.0f);
        displayShape(pShapes["dynamicShape112"],float3(11.0f,3.f,1.0f),systems["water"]->getSpacing()/4.0f);
        displayShape(pShapes["dynamicShape1123"],float3(14.0f,3.f,1.0f),systems["water"]->getSpacing()/8.0f);
        //displayShape(pShapes["dynamicShape11234"],float3(10.0f,0.0f,0.0f),systems["water"]->getSpacing()/16.0f);
        //}
        //glDisable(GL_DEPTH_TEST);

#endif

        glDisable(GL_MULTISAMPLE_ARB);
        if(renderMovie)
        {
            writeMovieFrame("image","./frames/");
            frameCounter++;
        }
        //showMass();

        glutSwapBuffers();

    }
    int TestApplication::writeMovieFrame(const char* filename, const char* dir)
    {
        GLubyte* image = new GLubyte[windowWidth*windowHeight*3];
	dout<<"width = "<<windowWidth<<" height = "<<windowHeight<<endl;
        stringstream s;
        s<<dir<<filename;
        s.fill('0');
        s.width(8);
        s<<right<<frameCounter;
        s<<".png";
        //sprintf(filename,"%s%s_%08d.png",render_dir,filename,frameCounter);
        glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, image);
        if (!stbi_write_png(s.str().c_str() , windowWidth, windowHeight,3,(void*)image,0))
        {
            delete[] image;
            cerr<<"failed to write image "<<filename<<endl;
            return -1;
        }
        delete[] image;
        return 0;
    }
    void TestApplication::DestroyCallback()
    {

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

        if (mouseButtons & 1)
        {
            rotation.x += dy * 0.2f;
            rotation.y += dx * 0.2f;
        }
        else if (mouseButtons & 4)
        {
            translation.z -= dy * 0.1f;
        }

        mousePos.x = x;
        mousePos.y = y;

        // set view matrix
        glutPostRedisplay();
    }
    void TestApplication::ResizeWindowCallback(int w, int h)
    {
        //avoid height = 0 this will cause divide by zero when calculating aspect ratio
        if (h==0)
        {
            h=1;
        }
        glViewport(0, 0, w, h);

        for(map<string,ParticleEffect*>::iterator i = effects.begin(); i!=effects.end(); i++)
        {
            i->second->setWindowDimensions(w,h);
        }

        // projection
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        windowWidth = w;
        windowHeight = h;
        //setFrustum();
        glutPostRedisplay();
    }
    void TestApplication::TimerCallback(int ms)
    {
        if(!paused)
        {
            glFinish();
            for(map<string,System*>::iterator i = systems.begin(); i!=systems.end(); i++)
            {
                i->second->acquireGLBuffers();
                i->second->update();
                i->second->interact();
            }

            for(map<string,System*>::iterator i = systems.begin(); i!=systems.end(); i++)
            {
                i->second->integrate();
                i->second->postProcess();
                i->second->releaseGLBuffers();
            }
            /*systems["water"]->acquireGLBuffers();
            systems["rb1"]->acquireGLBuffers();
            systems["water"]->update();
            systems["rb1"]->update();
            systems["water"]->interact();
            systems["rb1"]->interact();
            systems["water"]->integrate();
            systems["rb1"]->integrate();
            systems["water"]->postProcess();
            systems["rb1"]->postProcess();
            //streamline->addStreamLine(systems["water"]->getPositionBufferUnsorted(),systems["water"]->getColorBufferUnsorted(),systems["water"]->getNum());
            systems["water"]->releaseGLBuffers();
            systems["rb1"]->releaseGLBuffers();*/
        }
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

        glEnable(GL_LIGHTING);
        glPopAttrib();
    }
    void TestApplication::initGL()
    {
        // viewport
        glViewport(0, 0, windowWidth, windowHeight);

        // projection
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        //gluPerspective(60.0, (GLfloat)window_width / (GLfloat) window_height, 0.1, 100.0);
        //gluPerspective(fov, (GLfloat)window_width / (GLfloat) window_height, 0.3, 100.0);
        //gluPerspective(90.0, (GLfloat)window_width / (GLfloat) window_height, 0.1, 10000.0); //for lorentz

        // set view matrix
        glClearColor(.9f, .9f, .9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
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
			sysSettings[i]->SetSetting("rtps_path",path);
            //Fixme::This is hacky. I need to determine an efficient way to do simulation scaling
            //for rigid bodies to work well with sph.
            if(sysSettings[i]->GetSettingAs<string>("system")!="sph")
            {
                sysSettings[i]->SetSetting("smoothing_distance",systems["water"]->getSettings()->GetSettingAs<float>("smoothing_distance"));
                sysSettings[i]->SetSetting("simulation_scale",systems["water"]->getSettings()->GetSettingAs<float>("simulation_scale"));
            }
            systems[names[i]]=RTPS::generateSystemInstance(sysSettings[i],cli);
            dout<<"names[i] \'"<<names[i]<<"\'"<<endl;

        }
        gridMin = systems["water"]->getSettings()->GetSettingAs<float4>("domain_min");
        gridMax = systems["water"]->getSettings()->GetSettingAs<float4>("domain_max");
        for(map<string,System*>::iterator i = systems.begin(); i!=systems.end(); i++)
        {
            for(map<string,System*>::iterator j = systems.begin(); j!=systems.end(); j++)
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

    GLuint TestApplication::getWindowHeight() const {
        return windowHeight;
    }

    GLuint TestApplication::getWindowWidth() const {
        return windowWidth;
    }
    void TestApplication::get_bounding_box_for_node (const struct aiNode* nd,
        struct aiVector3D* minBB,
        struct aiVector3D* maxBB,
        struct aiMatrix4x4* trafo
    ){
        struct aiMatrix4x4 prev;
        unsigned int n = 0, t;

        prev = *trafo;
        aiMultiplyMatrix4(trafo,&nd->mTransformation);

        for (; n < nd->mNumMeshes; ++n) {
            const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
            for (t = 0; t < mesh->mNumVertices; ++t) {

                struct aiVector3D tmp = mesh->mVertices[t];
                aiTransformVecByMatrix4(&tmp,trafo);

                minBB->x = aisgl_min(minBB->x,tmp.x);
                minBB->y = aisgl_min(minBB->y,tmp.y);
                minBB->z = aisgl_min(minBB->z,tmp.z);

                maxBB->x = aisgl_max(maxBB->x,tmp.x);
                maxBB->y = aisgl_max(maxBB->y,tmp.y);
                maxBB->z = aisgl_max(maxBB->z,tmp.z);
            }
        }

        for (n = 0; n < nd->mNumChildren; ++n) {
            get_bounding_box_for_node(nd->mChildren[n],minBB,maxBB,trafo);
        }
        *trafo = prev;
    }

    // ----------------------------------------------------------------------------
    void TestApplication::get_bounding_box (struct aiVector3D* minBB, struct aiVector3D* maxBB)
    {
        struct aiMatrix4x4 trafo;
        aiIdentityMatrix4(&trafo);

        minBB->x = minBB->y = minBB->z =  1e10f;
        maxBB->x = maxBB->y = maxBB->z = -1e10f;
        get_bounding_box_for_node(scene->mRootNode,minBB,maxBB,&trafo);
    }

    // ----------------------------------------------------------------------------
    void TestApplication::color4_to_float4(const struct aiColor4D *c, float f[4])
    {
        f[0] = c->r;
        f[1] = c->g;
        f[2] = c->b;
        f[3] = c->a;
    }

    // ----------------------------------------------------------------------------
    void TestApplication::set_float4(float f[4], float a, float b, float c, float d)
    {
        f[0] = a;
        f[1] = b;
        f[2] = c;
        f[3] = d;
    }

    // ----------------------------------------------------------------------------
    void TestApplication::apply_material(const struct aiMaterial *mtl,Mesh* mesh)
    {
        float c[4];

        GLenum fill_mode;
        int ret1, ret2;
        struct aiColor4D diffuse;
        struct aiColor4D specular;
        struct aiColor4D ambient;
        struct aiColor4D emission;
        float shininess, strength, opacity;
        int two_sided;
        int wireframe;
        unsigned int maxVal;
	if(!mtl)
		cerr<<"No Material Found"<<endl;

        maxVal = 1;
        aiGetMaterialFloatArray(mtl,AI_MATKEY_OPACITY,&opacity,&maxVal);
        mesh->material.opacity = opacity;
        dout<<"Opacity: "<< opacity<<" Max "<<maxVal<<std::endl;
        set_float4(c, 0.8f, 0.8f, 0.8f, 1.0f);
        if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
            color4_to_float4(&diffuse, c);
        c[3]=opacity;
        memcpy(&mesh->material.diffuse.x,c,sizeof(float3));
        //glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, c);

        dout<<"red "<<c[0]<<" green  "<<c[1]<<" blue  "<<c[2]<<"  alpha  "<<c[3]<<std::endl;
        set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
        if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
            color4_to_float4(&specular, c);
        c[3]=opacity;
        memcpy(&mesh->material.specular.x,c,sizeof(float3));
        //glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);

        dout<<"red "<<c[0]<<" green  "<<c[1]<<" blue  "<<c[2]<<"  alpha  "<<c[3]<<std::endl;
        set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
        if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
            color4_to_float4(&ambient, c);
        c[3]=opacity;
        memcpy(&mesh->material.ambient.x,c,sizeof(float3));
        //glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, c);

        dout<<"red "<<c[0]<<" green  "<<c[1]<<" blue  "<<c[2]<<"  alpha  "<<c[3]<<std::endl;
        set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
        if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
            color4_to_float4(&emission, c);
        c[3]=opacity;
        //glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, c);

        dout<<"red "<<c[0]<<" green  "<<c[1]<<" blue  "<<c[2]<<"  alpha  "<<c[3]<<std::endl;
        maxVal = 1;
        ret1 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &maxVal);
        if(ret1 == AI_SUCCESS) {
            maxVal = 1;
            ret2 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &strength, &maxVal);
            if(ret2 == AI_SUCCESS)
            {
                //glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess * strength);
                mesh->material.shininess=shininess*strength;
            }
            else
            {
                //glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
                mesh->material.shininess=shininess;
            }
        }
        else {
            mesh->material.shininess=0.0f;
           // glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
            //set_float4(c, 0.0f, 0.0f, 0.0f, 0.0f);
            //c[3]=opacity;
            //glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);
        }

        maxVal = 1;
        if(AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_ENABLE_WIREFRAME, &wireframe, &maxVal))
            fill_mode = wireframe ? GL_LINE : GL_FILL;
        else
            fill_mode = GL_FILL;
        //glPolygonMode(GL_FRONT_AND_BACK, fill_mode);
    }
    void TestApplication::build_dynamic_shapes (const struct aiScene *sc, const struct aiNode* nd)
    {
        static unsigned int numshapes=0;
        unsigned int n = 0, t, i;

        float16 mat;
            aiMatrix4x4 m(1.0f,0.0f,0.0f,0.0f,
                          0.0f,1.0f,0.0f,0.0f,
                          0.0f,0.0f,1.0f,0.0f,
                          0.0f,0.0f,0.0f,1.0f);

        for (; n < nd->mNumMeshes; ++n) {
            const struct aiMesh* mesh = sc->mMeshes[nd->mMeshes[n]];
            float3 minCoord(FLT_MAX,FLT_MAX,FLT_MAX);
            float3 maxCoord(-FLT_MAX,-FLT_MAX,-FLT_MAX);
            Mesh* me=new Mesh();
            //dout<<"material index = "<<mesh->mMaterialIndex<<endl;
            apply_material(sc->mMaterials[mesh->mMaterialIndex],me);
            unsigned int* ibo = new unsigned int[mesh->mNumFaces*3];
            float* vbo = new float[mesh->mNumVertices*3];
            float* normals=NULL;
            if(mesh->HasNormals())
            {
                normals = new float[mesh->mNumVertices*3];
                me->hasNormals=true;
            }
            float* texcoords=NULL;
            if(mesh->HasTextureCoords(0))
            {
                texcoords = new float[mesh->mNumVertices*2];
                me->hasTexture=true;
            }
            for (t = 0; t < mesh->mNumFaces; ++t) {
                const struct aiFace* face = &mesh->mFaces[t];
                for(i = 0; i < face->mNumIndices; i++) {
                    unsigned int index = face->mIndices[i];
                    ibo[t*3+i]=index;
                    float x=mesh->mVertices[index].x;
                    float y=mesh->mVertices[index].y;
                    float z=mesh->mVertices[index].z;
                    vbo[index*3]=x;
                    vbo[index*3+1]=y;
                    vbo[index*3+2]=z;
                    if(normals)
                    {
                        normals[index*3]=mesh->mNormals[index].x;
                        normals[index*3+1]=mesh->mNormals[index].y;
                        normals[index*3+2]=mesh->mNormals[index].z;
                    }
                    if(texcoords)
                    {
                        texcoords[index*2]=mesh->mTextureCoords[0][index].x;
                        texcoords[index*2+1]=mesh->mTextureCoords[0][index].y;
                    }
            //        dout<<"index = "<<index<<"x = "<<x<<" "<<"y = "<<y<<" "<<"z = "<<z<<endl;
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
            }

            me->modelMat=m;
            me->vbo=createVBO(vbo,mesh->mNumVertices*3*sizeof(float),GL_ARRAY_BUFFER,GL_STATIC_DRAW );
            me->vboSize=mesh->mNumVertices;
            delete[] vbo;
            me->ibo=createVBO(ibo, mesh->mNumFaces*3*sizeof(int),GL_ELEMENT_ARRAY_BUFFER,GL_STATIC_DRAW );
            me->iboSize=mesh->mNumFaces*3;
            delete[] ibo;
            if(normals)
            {
                me->normalbo=createVBO(normals,mesh->mNumVertices*3*sizeof(float),GL_ARRAY_BUFFER,GL_STATIC_DRAW );
            }
            if(texcoords)
            {
                me->texCoordsbo=createVBO(texcoords,mesh->mNumVertices*2*sizeof(float),GL_ARRAY_BUFFER,GL_STATIC_DRAW );
            }

            stringstream s;
	    //I need too have a better way to handle more than 1 shape.
	    s<<"dynamicShape"<<numshapes++;
            dynamicMeshs[s.str()]=me;
            //dout<<"minCoord ("<<minCoord.x<<","<<minCoord.y<<","<<minCoord.z<<")"<<endl;
            //dout<<"maxCoord ("<<maxCoord.x<<","<<maxCoord.y<<","<<maxCoord.z<<")"<<endl;
            //Add padding equalt to spacing to ensure that all of the mesh is voxelized.
            float space = systems["rb1"]->getSpacing();
            ParticleShape* shape = new ParticleShape(minCoord,maxCoord,space);
            //NOTE: For Illustration use only.
            /*ParticleShape* shape1 = new ParticleShape(minCoord,maxCoord,space/2.0f);
            ParticleShape* shape2 = new ParticleShape(minCoord,maxCoord,space/4.0f);
            ParticleShape* shape3 = new ParticleShape(minCoord,maxCoord,space/8.0f);
            //ParticleShape* shape4 = new ParticleShape(minCoord,maxCoord,space/16.0f);
*/
            shape->voxelizeMesh(me->vbo,me->ibo,me->iboSize);
            /*shape1->voxelizeMesh(me->vbo,me->ibo,me->iboSize);
            shape2->voxelizeMesh(me->vbo,me->ibo,me->iboSize);
            shape3->voxelizeMesh(me->vbo,me->ibo,me->iboSize);
            //shape4->voxelizeMesh(me->vbo,me->ibo,me->iboSize);
            */
            //RenderUtils::write3DTextureToDisc(shape2->getVoxelTexture(),shape2->getVoxelResolution(),s.str().c_str());
            //shape->voxelizeSurface(me->vbo,me->ibo,me->iboSize);

            pShapes[s.str()]=shape;
            /*s<<1;
            pShapes[s.str()]=shape1;
            s<<2;
            pShapes[s.str()]=shape2;
            s<<3;
            pShapes[s.str()]=shape3;
            //s<<4;
            //pShapes[s.str()]=shape4;*/
            /*float3 dim = maxCoord-minCoord;
            float trans = (shape->getMaxDim()+shape->getMinDim())/2.0f;

            float16 modelview;
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            //glRotatef(rotation.x, 1.0, 0.0, 0.0);
            glTranslatef(trans, trans, trans);
            glRotatef(-180, 1.0, 0.0, 0.0);
            glRotatef(-90, 0.0, 0.0, 1.0);
            //glTranslatef(translation.x, translation.z, translation.y);
            glGetFloatv(GL_MODELVIEW_MATRIX,modelview.m);
            glPopMatrix();
            modelview.print("modelview");
            modelview.transpose();
            mat.print("mat before");
            mat = mat*modelview;
            mat.print("mat after");

            pShapes[s.str()]=shape;
            s<<1;
            pShapes[s.str()]=shape1;
            s<<2;
            pShapes[s.str()]=shape2;

            systems["rb1"]->addParticleShape(shape->getVoxelTexture(),shape->getMinDim(),shape->getMaxDim(),mat,shape->getVoxelResolution(),float4(0.0f,0.0f,0.0f,0.0f),float4(0.0f,0.0f,0.0f,1.0f),0.0f);*/
                //systems["rb1"]->addParticleShape(shape->getSurfaceTexture(),shape->getMinDim(),shape->getMaxDim(),mat,shape->getVoxelResolution(),float4(0.0f,0.0f,0.0f,0.0f),float4(0.0f,0.0f,0.0f,1.0f),0.0f);
        }

        // draw all children
        for (n = 0; n < nd->mNumChildren; ++n) {
            build_dynamic_shapes(sc, nd->mChildren[n]);
        }

    }

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

    void TestApplication::build_shapes (const struct aiScene *sc, const struct aiNode* nd, struct aiMatrix4x4 parentTransform)
    {
        unsigned int n = 0, t,i;

        struct aiMatrix4x4 m = nd->mTransformation;
        aiMultiplyMatrix4(&m,&parentTransform);

        // update transform
        aiTransposeMatrix4(&m);

        float16 mat;
        memcpy(&mat,&m,sizeof(float16));

        for (; n < nd->mNumMeshes; ++n) {
            const struct aiMesh* mesh = sc->mMeshes[nd->mMeshes[n]];

            //dout<<"num faces "<<mesh->mNumFaces<<endl;
            float3 minCoord(FLT_MAX,FLT_MAX,FLT_MAX);
            float3 maxCoord(-FLT_MAX,-FLT_MAX,-FLT_MAX);
            Mesh* me=new Mesh();
            //dout<<"material index = "<<mesh->mMaterialIndex<<endl;
            apply_material(sc->mMaterials[mesh->mMaterialIndex],me);
            unsigned int* ibo = new unsigned int[mesh->mNumFaces*3];
            float* vbo = new float[mesh->mNumVertices*3];
            float* normals=NULL;
            if(mesh->HasNormals())
            {
                normals = new float[mesh->mNumVertices*3];
                me->hasNormals=true;
            }
            float* texcoords=NULL;
            if(mesh->HasTextureCoords(0))
            {
                texcoords = new float[mesh->mNumVertices*2];
                me->hasTexture=true;
            }
            for (t = 0; t < mesh->mNumFaces; ++t) {
                const struct aiFace* face = &mesh->mFaces[t];
                for(i = 0; i < face->mNumIndices; i++) {
                    unsigned int index = face->mIndices[i];
                    ibo[t*3+i]=index;
                    float x=mesh->mVertices[index].x;
                    float y=mesh->mVertices[index].y;
                    float z=mesh->mVertices[index].z;
                    vbo[index*3]=x;
                    vbo[index*3+1]=y;
                    vbo[index*3+2]=z;
                    if(normals)
                    {
                        normals[index*3]=mesh->mNormals[index].x;
                        normals[index*3+1]=mesh->mNormals[index].y;
                        normals[index*3+2]=mesh->mNormals[index].z;
                    }
                    if(texcoords)
                    {
                        texcoords[index*2]=mesh->mTextureCoords[0][index].x;
                        texcoords[index*2+1]=mesh->mTextureCoords[0][index].y;
                    }
            //        dout<<"index = "<<index<<"x = "<<x<<" "<<"y = "<<y<<" "<<"z = "<<z<<endl;
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
            }
            me->modelMat=m;
            me->vbo=createVBO(vbo,mesh->mNumVertices*3*sizeof(float),GL_ARRAY_BUFFER,GL_STATIC_DRAW );
            me->vboSize=mesh->mNumVertices;
            delete[] vbo;
            me->ibo=createVBO(ibo, mesh->mNumFaces*3*sizeof(int),GL_ELEMENT_ARRAY_BUFFER,GL_STATIC_DRAW );
            me->iboSize=mesh->mNumFaces*3;
            delete[] ibo;
            if(normals)
            {
                me->normalbo=createVBO(normals,mesh->mNumVertices*3*sizeof(float),GL_ARRAY_BUFFER,GL_STATIC_DRAW );
            }
            if(texcoords)
            {
                me->texCoordsbo=createVBO(texcoords,mesh->mNumVertices*2*sizeof(float),GL_ARRAY_BUFFER,GL_STATIC_DRAW );
            }

            stringstream s;
            s<<"test"<<mesh->mNumFaces;
            meshs[s.str()]=me;
            //dout<<"minCoord ("<<minCoord.x<<","<<minCoord.y<<","<<minCoord.z<<")"<<endl;
            //dout<<"maxCoord ("<<maxCoord.x<<","<<maxCoord.y<<","<<maxCoord.z<<")"<<endl;
            //Add padding equalt to spacing to ensure that all of the mesh is voxelized.
            /*float space = systems["rb1"]->getSpacing()/2.f;
            float3 adjminCoord=float3(minCoord.x-space,minCoord.y-space,minCoord.z-space);
            float3 adjmaxCoord=float3(maxCoord.x+space,maxCoord.y+space,maxCoord.z+space);
            space = systems["rb1"]->getSpacing();
            ParticleShape* shape = new ParticleShape(adjminCoord,adjmaxCoord,space);*/
            float space = systems["rb1"]->getSpacing();
            ParticleShape* shape = new ParticleShape(minCoord,maxCoord,space);


            shape->voxelizeMesh(me->vbo,me->ibo,me->iboSize);
            //RenderUtils::write3DTextureToDisc(shape->getVoxelTexture(),shape->getVoxelResolution(),s.str().c_str());
            //shape->voxelizeSurface(me->vbo,me->ibo,me->iboSize);
            s<<"surface";
            //RenderUtils::write3DTextureToDisc(shape->getSurfaceTexture(),shape->getVoxelResolution(),s.str().c_str());
            float trans = (shape->getMaxDim()+shape->getMinDim())/2.0f;
            /*dout<<"mesh name = "<<s.str()<<endl;
            dout<<"maxCoord dim = "<<shape->getMaxDim()<<endl;
            dout<<"minCoord dim = "<<shape->getMinDim()<<endl;
            dout<<"Trans = "<<trans<<endl;
            dout<<"minCoord ("<<shape->getMin().x<<","<<shape->getMin().y<<","<<shape->getMin().z<<")"<<endl;
            dout<<"voxel res = "<<shape->getVoxelResolution()<<endl;
            dout<<"spacing = "<<space<<endl;*/
            float3 dim = maxCoord-minCoord;



            float16 modelview;
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();
            //glRotatef(rotation.x, 1.0, 0.0, 0.0);
            glTranslatef(trans, trans, trans);
            glRotatef(-180, 1.0, 0.0, 0.0);
            glRotatef(-90, 0.0, 0.0, 1.0);
            //glTranslatef(translation.x, translation.z, translation.y);
            glGetFloatv(GL_MODELVIEW_MATRIX,modelview.m);
            glPopMatrix();
            modelview.print("modelview");
            modelview.transpose();
            mat.print("mat before");
            mat = mat*modelview;
            mat.print("mat after");
            pShapes[s.str()]=shape;
           //Debug!!****
            //if(mesh->mNumFaces<50)


                systems["rb1"]->addParticleShape(shape->getVoxelTexture(),shape->getMinDim(),shape->getMaxDim(),mat,shape->getVoxelResolution(),float4(0.0f,0.0f,0.0f,0.0f),float4(0.0f,0.0f,0.0f,1.0f),0.0f);
                //systems["rb1"]->addParticleShape(shape->getSurfaceTexture(),shape->getMinDim(),shape->getMaxDim(),mat,shape->getVoxelResolution(),float4(0.0f,0.0f,0.0f,0.0f),float4(0.0f,0.0f,0.0f,1.0f),0.0f);
        }

        // draw all children
        for (n = 0; n < nd->mNumChildren; ++n) {
            build_shapes(sc, nd->mChildren[n],m);
        }

    }

    // ----------------------------------------------------------------------------
    void TestApplication::recursive_render (const struct aiScene *sc, const struct aiNode* nd)
	{
        unsigned int n = 0, t,i=0;
        struct aiMatrix4x4 m = nd->mTransformation;

        // update transform
        aiTransposeMatrix4(&m);
        glPushMatrix();
        glMultMatrixf((float*)&m);

        // draw all meshes assigned to this node
        for (; n < nd->mNumMeshes; ++n) {
            const struct aiMesh* mesh = sc->mMeshes[nd->mMeshes[n]];

            //apply_material(sc->mMaterials[mesh->mMaterialIndex]);

            if(mesh->mNormals == NULL) {
                glDisable(GL_LIGHTING);
            } else {
                glEnable(GL_LIGHTING);
            }


            for (t = 0; t < mesh->mNumFaces; ++t) {
                const struct aiFace* face = &mesh->mFaces[t];
                GLenum face_mode;

                switch(face->mNumIndices) {
                    case 1: face_mode = GL_POINTS; break;
                    case 2: face_mode = GL_LINES; break;
                    case 3: face_mode = GL_TRIANGLES; break;
                    default: face_mode = GL_POLYGON; break;
                }

                //dout<<"Face mode = "<<face_mode<<" GL_TRIANGLES = "<<GL_TRIANGLES<<endl;
                glBegin(face_mode);


                for(i = 0; i < face->mNumIndices; i++) {
                    int index = face->mIndices[i];
                    if(mesh->mColors[0] != NULL)
                    {
                        glColor4fv((GLfloat*)&mesh->mColors[0][index]);
                    }
                    if(mesh->mNormals != NULL)
                        glNormal3fv(&mesh->mNormals[index].x);
                    glVertex3fv(&mesh->mVertices[index].x);
                    //dout<<"index = "<<index<<"x = "<<mesh->mVertices[index].x<<" "<<"y = "<<mesh->mVertices[index].y<<" "<<"z = "<<mesh->mVertices[index].z<<endl;
                }

                glEnd();
            }

        }

        // draw all children
        for (n = 0; n < nd->mNumChildren; ++n) {
            recursive_render(sc, nd->mChildren[n]);
        }

        glPopMatrix();
    }
    void TestApplication::display(bool blend)
    {
        float tmp;
        glEnable(GL_NORMALIZE);
        glEnable(GL_CULL_FACE);
        glDisable(GL_LIGHTING);
        if(blend)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        }
        for(map<string,Mesh*>::iterator i = meshs.begin(); i!=meshs.end(); i++)
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
    void TestApplication::loadMeshScene(string& filename)
    {
        dynamicMeshScene = aiImportFile(filename.c_str(),aiProcessPreset_TargetRealtime_MaxQuality);
	if(!dynamicMeshScene)
            cerr<<"Scene file couldn't be imported from: "<<filename<<endl;
    }
    void TestApplication::loadScene(string& filename)
    {
        // we are taking one of the postprocessing presets to avoid
        // spelling out 20+ single postprocessing flags here.
        scene = aiImportFile(filename.c_str(),aiProcessPreset_TargetRealtime_MaxQuality);

        if (scene) {
            get_bounding_box(&scene_min,&scene_max);
            scene_center.x = (scene_min.x + scene_max.x) / 2.0f;
            scene_center.y = (scene_min.y + scene_max.y) / 2.0f;
            scene_center.z = (scene_min.z + scene_max.z) / 2.0f;
        }
        else
        {
            cerr<<"Scene file couldn't be imported from: "<<filename<<endl;
        }

    }
};
