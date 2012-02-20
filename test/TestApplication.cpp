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
#include <RTPS.h>
#include "../rtpslib/render/SSEffect.h"
#include <../rtpslib/system/ParticleRigidBody.h>

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
    TestApplication::TestApplication(istream& is)
    {
        glewInit();
        GLboolean bGLEW = glewIsSupported("GL_VERSION_2_0 GL_ARB_pixel_buffer_object");
        windowWidth=640;
        windowWidth=480;
        cli = new CL();

        renderType="default";
        readParamFile(is);
        initGL();
        //Fixme: This is a bad way to make sure the directory is correct.
        RenderSettings rs;
        //rs.blending=false;
        rs.blending=false;
        float nf[2];
        glGetFloatv(GL_DEPTH_RANGE,nf);
        rs.near = nf[0];
        rs.far = nf[1];
        dout<<"near = "<<rs.near<<endl;
        dout<<"far = "<<rs.far<<endl;
        dout<<"spacing = "<<systems["water"]->getSpacing()<<endl;
        rs.particleRadius = systems["water"]->getSpacing()*20.f;
        rs.windowWidth=windowWidth;
        rs.windowHeight=windowHeight;
        lib = new ShaderLibrary();
        lib->initializeShaders(GLSL_BIN_DIR);
        effects["default"]=new ParticleEffect(rs,*lib);
        //effects["sprite"]=new ParticleEffect();
        rs.blending=true;
        rs.particleRadius =systems["water"]->getSpacing()*.4f;
        effects["ssfr"]=new SSEffect(rs, *lib);
        translation.x = -2.00f;
        translation.y = -2.70f;//300.f;
        translation.z = 3.50f;
        rotation.x=0.0f;
        rotation.y=0.0f;
        mass=1.0f;
        string blendfile = "demo_scene.obj";

        renderVelocity=false;
        paused=false;
        scene=NULL;
        scene_list=0;
        loadScene(blendfile);
        build_shapes(scene, scene->mRootNode);
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
            case 'e': //dam break
            {
                nn = systems["water"]->getSettings()->GetSettingAs<unsigned int>("max_num_particles")/2;
                float4 col1 = float4(0.05, 0.15, 8., 0.1);
                systems["water"]->addBox(nn, gridMin+float4(0.5f,0.5f,0.5f,1.0f), gridMax-float4(0.5f,0.5f,0.5f,1.0f), false,col1);
                //ps2->system->addBox(nn, min, max, false);
                return;
            }
            case 'g':
            {
                //nn = 16384;
                nn = systems["water"]->getSettings()->GetSettingAs<unsigned int>("max_num_particles")/8;
                float4 min = float4(2.5f, 2.5f, 2.5f, 1.0f);
                float4 max = float4(7.5f, 7.5f, 7.5f, 1.0f);
                float4 col1 = float4(0., 0., 1., 0.05);
                systems["water"]->addBox(nn, min, max, false,col1);
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
                float4 center = float4(gridMin.x+3.0f, gridMin.y+3.0f,gridMax.z-2.0f,1.0f);
                float4 col1 = float4(0.05, 0.15, 8., 0.1);
                float4 velocity(0., 0., -2.f, 0);
                float radius= 3.0f;
                //sph sets spacing and multiplies by radius value
                systems["water"]->addHose(50000, center, velocity,radius, col1);
                return;
            }
            case 'n':
                //render_movie=!render_movie;
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
                float4 col1 = float4(0.0, 0.8, 0.2, 1.);
                float4 size = float4(1.,1.,1.,0.f);
                float4 position = float4(gridMin.x+0.1f, gridMin.y+0.1f,gridMin.z+.1f,1.0f);
                systems["rb1"]->addBox(1000, position, float4(gridMax.x-0.1f,gridMax.y-0.1f,gridMin.z+.5f,1.0f), false, col1,0.0f);
                dout<<"Here"<<endl;
                position = float4(gridMin.x+0.1f, gridMin.y+0.1f,gridMin.z+0.1f,1.0f);
                systems["rb1"]->addBox(1000, position, float4(gridMin.x+0.5f,gridMax.y-0.1f,gridMax.z-.1f,1.0f), false, col1,0.0f);
                dout<<"Here"<<endl;
                position = float4(gridMin.x+0.1f, gridMin.y+0.1f,gridMin.z+0.1f,1.0f);
                systems["rb1"]->addBox(1000, position, float4(gridMax.x-0.1f,gridMin.y+0.5f,gridMax.z-.1f,1.0f), false, col1,0.0f);
                dout<<"Here"<<endl;
                position = float4(gridMax.x-0.5f, gridMin.y+0.1f,gridMin.z+0.1f,1.0f);
                systems["rb1"]->addBox(1000, position, float4(gridMax.x-0.1f,gridMax.y-0.1f,gridMax.z-.1f,1.0f), false, col1,0.0f);
                dout<<"Here"<<endl;
                position = float4(gridMin.x+0.1f, gridMax.y-0.5f,gridMin.z+0.1f,1.0f);
                systems["rb1"]->addBox(1000, position, float4(gridMax.x-0.1f,gridMax.y-0.1f,gridMax.z-.1f,1.0f), false, col1,0.0f);
                dout<<"Here"<<endl;
                return;
            }
            case 'R': //drop a rectangle
                {

                    float4 col1 = float4(0.5, 0.9, 0.0, 1.);

                    float4 size = float4(1.,1.,1.,0.f);
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
                    float4 col1 = float4(0.5, 0.9, 0.0, 1.);
                    float size = 1.0f;
                    size=size*sizeScale;
                    float4 mid = (gridMax-gridMin);
                    mid = mid/2.0f;
                    mid.w = 0.0f;

                    systems["rb1"]->addBall(1000, mid, size,false, col1,mass);
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
            case 'w':
                translation.z -= 0.1;
                break;
            case 'a':
                translation.x += 0.1;
                break;
            case 's':
                translation.z += 0.1;
                break;
            case 'd':
                translation.x -= 0.1;
                break;
            case 'z':
                translation.y += 0.1;
                break;
            case 'x':
                translation.y -= 0.1;
                break;
            case '+':
                mass*=10.0f;
                break;
            case '-':
                mass/=10.0f;
                break;
            case '[':
                sizeScale+=0.5f;
                break;
            case ']':
                sizeScale-=0.5f;
                break;
            default:
                return;
        }

        glutPostRedisplay();
    }
    void TestApplication::RenderCallback()
    {

        glClearColor(.9, .9, .9, 1.0);
        //ps->system->sprayHoses();;
        /*if(!voxelized)
        {
           float3 min(FLT_MAX,FLT_MAX,FLT_MAX);
            float3 max(-FLT_MAX,-FLT_MAX,-FLT_MAX);
            for(int i = 0; i<BUNNY_NUM_VERTICES; i++)
            {
                float x = gVerticesBunny[(i*3)];
                float y = gVerticesBunny[(i*3)+1];
                float z = gVerticesBunny[(i*3)+2];
                if(x<min.x)
                    min.x=x;
                if(x>max.x)
                    max.x=x;
                if(y<min.y)
                    min.y=y;
                if(y>max.y)
                    max.y=y;
                if(z<min.z)
                    min.z=z;
                if(z>max.z)
                    max.z=z;
            }
            cout<<"min ("<<min.x<<","<<min.y<<","<<min.z<<")"<<endl;
            cout<<"max ("<<max.x<<","<<max.y<<","<<max.z<<")"<<endl;
            bunnyShape = new ParticleShape(min,max,systems["rb1"]->getSpacing(),3.0f);
            bunnyShape->voxelizeMesh(bunnyVBO,bunnyIBO,3*BUNNY_NUM_TRIANGLES);
            //write3DTextureToDisc(bunnyShape->getVoxelTexture(),bunnyShape->getVoxelResolution(),"bunnytex");
            voxelized=true;
        }*/
        glEnable(GL_DEPTH_TEST);
        /*if (stereo_enabled)
        {
            render_stereo();
        }
        else
        {*/
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


            /*glBindBuffer(GL_ARRAY_BUFFER, bunnyVBO);
            glVertexPointer(3, GL_FLOAT, 0, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bunnyIBO);
            glEnableClientState( GL_VERTEX_ARRAY );
            glDrawElements(GL_TRIANGLES,3*BUNNY_NUM_TRIANGLES,GL_UNSIGNED_INT,0);
            glDisableClientState( GL_VERTEX_ARRAY );
             */

            //RenderUtils::renderBox(gridMin,gridMax,float4(0.0f,1.0,0.0f,1.0f));
            //FIXME: Have a method to give renderType to each System. That way we can have different
            //Systems with the different effects.
            for(map<string,System*>::iterator i = systems.begin(); i!=systems.end(); i++)
            {
                if(renderVelocity)
                {
                    effects[renderType]->renderVector(i->second->getPosVBO(),i->second->getVelocityVBO(),i->second->getNum());
                }
                effects[renderType]->render(i->second->getPosVBO(),i->second->getColVBO(),i->second->getNum());
                //FIXME:This is a horrible way of doing this!!
                if(i->first=="rb1")
                {
                    ParticleRigidBody* prb=((ParticleRigidBody*)i->second);
                    effects[renderType]->render(prb->getStaticVBO(),prb->getColVBO(),prb->getStaticNum());
                }
            }
            /*if(render_movie)
            {
                //write_movie_frame("image");
            }*/

        //}
        glDisable(GL_DEPTH_TEST);


        /*if(render_movie)
        {
            //frame_counter++;
        }*/
        //showMass();

        glEnable(GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        display();
        glDisable(GL_BLEND);
        glutSwapBuffers();

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
            rotation.x += dy * 0.2;
            rotation.y += dx * 0.2;
        }
        else if (mouseButtons & 4)
        {
            translation.z -= dy * 0.1;
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
        glClearColor(.9, .9, .9, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

    void TestApplication::readParamFile(std::istream& is)
    {
        ParamParser p;
        vector<RTPSSettings*> sysSettings;
        vector<string> names;
        p.readParameterFile(is,sysSettings ,names );
        for(unsigned int i = 0; i<sysSettings.size(); i++)
        {
            #ifdef WIN32
                sysSettings[i]->SetSetting("rtps_path",".");
            #else
                sysSettings[i]->SetSetting("rtps_path","./bin");
            #endif
            //Fixme::This is hacky. I need to determine an efficient way to do simulation scaling
            //for rigid bodies to work well with sph.
            if(sysSettings[i]->GetSettingAs<string>("system")=="rigidbody")
            {
                sysSettings[i]->SetSetting("smoothing_distance",systems["water"]->getSettings()->GetSettingAs<float>("smoothing_distance"));
                sysSettings[i]->SetSetting("simulation_scale",systems["water"]->getSettings()->GetSettingAs<float>("simulation_scale"));
            }
            systems[names[i]]=RTPS::generateSystemInstance(sysSettings[i],cli);

        }
        gridMin = systems["water"]->getSettings()->GetSettingAs<float4>("domain_min");
        gridMax = systems["water"]->getSettings()->GetSettingAs<float4>("domain_max");
        for(map<string,System*>::iterator i = systems.begin(); i!=systems.end(); i++)
        {
            for(map<string,System*>::iterator j = systems.begin(); j!=systems.end(); j++)
            {
                if(i==j)
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
        struct aiVector3D* min,
        struct aiVector3D* max,
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

                min->x = aisgl_min(min->x,tmp.x);
                min->y = aisgl_min(min->y,tmp.y);
                min->z = aisgl_min(min->z,tmp.z);

                max->x = aisgl_max(max->x,tmp.x);
                max->y = aisgl_max(max->y,tmp.y);
                max->z = aisgl_max(max->z,tmp.z);
            }
        }

        for (n = 0; n < nd->mNumChildren; ++n) {
            get_bounding_box_for_node(nd->mChildren[n],min,max,trafo);
        }
        *trafo = prev;
    }

    // ----------------------------------------------------------------------------
    void TestApplication::get_bounding_box (struct aiVector3D* min, struct aiVector3D* max)
    {
        struct aiMatrix4x4 trafo;
        aiIdentityMatrix4(&trafo);

        min->x = min->y = min->z =  1e10f;
        max->x = max->y = max->z = -1e10f;
        get_bounding_box_for_node(scene->mRootNode,min,max,&trafo);
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
    void TestApplication::apply_material(const struct aiMaterial *mtl)
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
        unsigned int max;

        max = 1;
        aiGetMaterialFloatArray(mtl,AI_MATKEY_OPACITY,&opacity,&max);
        dout<<"Opacity: "<< opacity<<" Max "<<max<<std::endl;
        set_float4(c, 0.8f, 0.8f, 0.8f, 1.0f);
        if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
            color4_to_float4(&diffuse, c);
        c[3]=opacity;
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, c);

        dout<<"red "<<c[0]<<" green  "<<c[1]<<" blue  "<<c[2]<<"  alpha  "<<c[3]<<std::endl;
        set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
        if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
            color4_to_float4(&specular, c);
        c[3]=opacity;
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);

        dout<<"red "<<c[0]<<" green  "<<c[1]<<" blue  "<<c[2]<<"  alpha  "<<c[3]<<std::endl;
        set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
        if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
            color4_to_float4(&ambient, c);
        c[3]=opacity;
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, c);

        dout<<"red "<<c[0]<<" green  "<<c[1]<<" blue  "<<c[2]<<"  alpha  "<<c[3]<<std::endl;
        set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
        if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
            color4_to_float4(&emission, c);
        c[3]=opacity;
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, c);

        dout<<"red "<<c[0]<<" green  "<<c[1]<<" blue  "<<c[2]<<"  alpha  "<<c[3]<<std::endl;
        max = 1;
        ret1 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
        if(ret1 == AI_SUCCESS) {
            max = 1;
            ret2 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &strength, &max);
            if(ret2 == AI_SUCCESS)
                glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess * strength);
            else
                glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
        }
        else {
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
            set_float4(c, 0.0f, 0.0f, 0.0f, 0.0f);
            c[3]=opacity;
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);
        }

        max = 1;
        if(AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_ENABLE_WIREFRAME, &wireframe, &max))
            fill_mode = wireframe ? GL_LINE : GL_FILL;
        else
            fill_mode = GL_FILL;
        glPolygonMode(GL_FRONT_AND_BACK, fill_mode);

        max = 1;
        if((AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_TWOSIDED, &two_sided, &max)) && two_sided)
            glDisable(GL_CULL_FACE);
        else
            glEnable(GL_CULL_FACE);
    }
    void TestApplication::build_shapes (const struct aiScene *sc, const struct aiNode* nd)
    {
        int i;
        unsigned int n = 0, t;

        struct aiMatrix4x4 m = nd->mTransformation;


        // update transform
        aiTransposeMatrix4(&m);

        float16 mat;
        memcpy(&mat,&m,sizeof(float16));
        /*for(int i =0;i<16;i++){
            mat.m[i]=m[0][i];
            dout<<"mat i "<<i<<" = "<<mat.m[i]<<endl;
        }*/
        // draw all meshes assigned to this node
        for (; n < nd->mNumMeshes; ++n) {
            const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
            dout<<"num faces "<<mesh->mNumFaces<<endl;
            Mesh* me=new Mesh();
            unsigned int* ibo = new unsigned int[mesh->mNumFaces*3];
            for (t = 0; t < mesh->mNumFaces; ++t) {
                const struct aiFace* face = &mesh->mFaces[t];
                for(i = 0; i < face->mNumIndices; i++) {
                    ibo[t*3+i]=face->mIndices[i];
                }
            }
            float* vbo = new float[mesh->mNumVertices*3];
            float3 min(FLT_MAX,FLT_MAX,FLT_MAX);
            float3 max(-FLT_MAX,-FLT_MAX,-FLT_MAX);
            for(int i = 0; i<mesh->mNumVertices; i++)
            {
                float x = mesh->mVertices[i].x;
                float y = mesh->mVertices[i].y;
                float z = mesh->mVertices[i].z;
                vbo[i*3]=x;
                vbo[i*3+1]=y;
                vbo[i*3+2]=z;
                if(x<min.x)
                    min.x=x;
                if(x>max.x)
                    max.x=x;
                if(y<min.y)
                    min.y=y;
                if(y>max.y)
                    max.y=y;
                if(z<min.z)
                    min.z=z;
                if(z>max.z)
                    max.z=z;
            }
            me->vbo=createVBO(vbo,mesh->mNumVertices*3*sizeof(float),GL_ARRAY_BUFFER,GL_STATIC_DRAW );
            me->vboSize=mesh->mNumVertices;
            delete[] vbo;
            me->ibo=createVBO(ibo, mesh->mNumFaces*3*sizeof(int),GL_ELEMENT_ARRAY_BUFFER,GL_STATIC_DRAW );
            me->iboSize=mesh->mNumFaces*3;
            delete[] ibo;
            stringstream s;
            s<<"test"<<me->vbo<<endl;
            meshs[s.str()]=me;
            dout<<"min ("<<min.x<<","<<min.y<<","<<min.z<<")"<<endl;
            dout<<"max ("<<max.x<<","<<max.y<<","<<max.z<<")"<<endl;
            ParticleShape* shape = new ParticleShape(min,max,systems["rb1"]->getSpacing(),1.0f);
            shape->voxelizeMesh(me->vbo,me->ibo,me->iboSize);
            dout<<"mesh name = "<<s<<endl;
            pShapes[s.str()]=shape;
            systems["rb1"]->addParticleShape(shape->getVoxelTexture(),shape->getMaxDim(),float4(shape->getMin(),0.0f),mat,shape->getVoxelResolution(),float4(0.0f,0.0f,0.0f,0.0f),float4(0.0f,0.0f,0.0f,1.0f),0.0f);
        }

        // draw all children
        for (n = 0; n < nd->mNumChildren; ++n) {
            build_shapes(sc, nd->mChildren[n]);
        }

    }

    // ----------------------------------------------------------------------------
    void TestApplication::recursive_render (const struct aiScene *sc, const struct aiNode* nd)
    {
        int i;
        unsigned int n = 0, t;
        struct aiMatrix4x4 m = nd->mTransformation;

        // update transform
        aiTransposeMatrix4(&m);
        glPushMatrix();
        glMultMatrixf((float*)&m);

        // draw all meshes assigned to this node
        for (; n < nd->mNumMeshes; ++n) {
            const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];

            apply_material(sc->mMaterials[mesh->mMaterialIndex]);

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
    void TestApplication::display(void)
    {
        float tmp;
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);    // Uses default lighting parameters

        glEnable(GL_DEPTH_TEST);

        glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
        glEnable(GL_NORMALIZE);

        // XXX docs say all polygons are emitted CCW, but tests show that some aren't.
        if(getenv("MODEL_IS_BROKEN"))
            glFrontFace(GL_CW);

        glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);


        if(scene_list == 0) {
            scene_list = glGenLists(1);
            glNewList(scene_list, GL_COMPILE);
                // now begin at the root node of the imported data and traverse
                // the scenegraph by multiplying subsequent local transforms
                // together on GL's matrix stack.
            recursive_render(scene, scene->mRootNode);
            glEndList();
        }

        glCallList(scene_list);
        glDisable(GL_LIGHTING);
        glDisable(GL_NORMALIZE);
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
