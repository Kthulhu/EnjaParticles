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
#include <QtGui>
#include <QGLWidget>
#include "ParamParser.h"
#include "../rtpslib/RTPS.h"
#include "../rtpslib/render/SSEffect.h"
#include "../rtpslib/render/MeshEffect.h"
#include <../rtpslib/system/ParticleRigidBody.h>
#include <../rtpslib/system/SPH.h>
#include <../rtpslib/system/FLOCK.h>
 #include <math.h>
#include <sstream>
#include <float.h>
 #include "glwidget.h"

 GLWidget::GLWidget(QWidget *parent)
     : QGLWidget(parent)
 {
     glewInit();
     GLboolean bGLEW = glewIsSupported("GL_VERSION_2_0 GL_ARB_pixel_buffer_object");
     cli = new CL();
     renderType="default";
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
        //rs.particleRadius = systems["water"]->getSpacing()*20.f;
        //rs.windowWidth=windowWidth;
        //rs.windowHeight=windowHeight;
        lib = new ShaderLibrary();
	string shaderpath=path+"/shaders";
        lib->initializeShaders(shaderpath);
        effects["default"]=new ParticleEffect(rs,*lib);
        //effects["sprite"]=new ParticleEffect();
        rs.blending=true;
        //rs.particleRadius =systems["water"]->getSpacing()*.6f;
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
        //string scenefile = path+"/demo_scene.obj";

        renderVelocity=false;
        paused=false;
        //scene=NULL;
        scene_list=0;
        //loadScene(scenefile);
        renderMovie=false;
        frameCounter=0;
        string meshesfile = path+"/demo_mesh_scene.obj";
        //loadMeshScene(meshesfile);
        //build_shapes(scene, scene->mRootNode);
        //build_dynamic_shapes(dynamicMeshScene, dynamicMeshScene->mRootNode);
        environTex = RenderUtils::loadCubemapTexture(path+"/cubemaps/");

     fov=65.0f;
     near=0.3f;
     far=1000.0f;
     QTimer *timer = new QTimer(this);
     connect(timer, SIGNAL(timeout()), this, SLOT(advanceGears()));
     timer->start(20);
 }

 GLWidget::~GLWidget()
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
     makeCurrent();
 }

 void GLWidget::initializeGL()
 {
     static const GLfloat lightPos[4] = { 5.0f, 5.0f, 10.0f, 1.0f };
     static const GLfloat reflectance1[4] = { 0.8f, 0.1f, 0.0f, 1.0f };
     static const GLfloat reflectance2[4] = { 0.0f, 0.8f, 0.2f, 1.0f };
     static const GLfloat reflectance3[4] = { 0.2f, 0.2f, 1.0f, 1.0f };

     glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
     glEnable(GL_LIGHTING);
     glEnable(GL_LIGHT0);
     glEnable(GL_DEPTH_TEST);
     glEnable(GL_NORMALIZE);
     //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
     // viewport
        glViewport(0, 0, width(), height());

        // projection
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(fov, width()/(double)height(), near, far);
        // set view matrix
        glClearColor(.9f, .9f, .9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
 }

 void GLWidget::paintGL()
 {
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
            /*ParticleRigidBody* rbsys = (ParticleRigidBody*)systems["rb1"];
            meshRenderer->renderInstanced(dynamicMeshs["dynamicShape0"],rbsys->getComPosVBO(),rbsys->getComRotationVBO(),rbsys->getNum(),light);
            if(systems.find("flock1")!=systems.end())
            {
                //dout<<"flock------------------"<<endl;
                FLOCK* flock = (FLOCK*)systems["flock1"];
                //effects[renderType]->render(flock->getPosVBO(),flock->getColVBO(),flock->getNum());
                meshRenderer->renderInstanced(dynamicMeshs["dynamicShape1"],flock->getPosVBO(),flock->getRotationVBO(),flock->getNum(),light);
            }
            display(false);
*/
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
            /*SPH* sph = (SPH*)systems["water"];
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            glBlendFunc(GL_ONE,GL_ONE);
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
            }*/
            display(true);
#else
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
	updateGL();
 }

 void GLWidget::resizeGL(int width, int height)
 {
     int side = qMin(width, height);
     glViewport(0, 0, width, height);
     for(map<string,ParticleEffect*>::iterator i = effects.begin(); i!=effects.end(); i++)
        {
            i->second->setWindowDimensions(w,h);
        }
     glMatrixMode(GL_PROJECTION);
     glLoadIdentity();
     gluPerspective(fov, width/(double)height, near, far);
 }

 void GLWidget::mousePressEvent(QMouseEvent *event)
 {
     
 }

 void GLWidget::mouseMoveEvent(QMouseEvent *event)
 {
     
 }

void GLWidget::keyPressEvent(QKeyEvent *event)
{

}
void GLWidget::readParamFile(std::istream& is, QString path)
{

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
void GLWidget::createParticleShape(Mesh* mesh)
{
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
        float* pos = new float[mesh->vbosize*3];
        float3 minCoord(FLT_MAX,FLT_MAX,FLT_MAX);
        float3 maxCoord(-FLT_MAX,-FLT_MAX,-FLT_MAX);
        for(int i = 0; i < mesh->vbosize*3; i++)
	{
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
            float space = systems["rb1"]->getSpacing();
            ParticleShape* shape = new ParticleShape(minCoord,maxCoord,space);


            shape->voxelizeMesh(me->vbo,me->ibo,me->iboSize);
            //RenderUtils::write3DTextureToDisc(shape->getVoxelTexture(),shape->getVoxelResolution(),s.str().c_str());
            //shape->voxelizeSurface(me->vbo,me->ibo,me->iboSize);
            s<<"surface";
            //RenderUtils::write3DTextureToDisc(shape->getSurfaceTexture(),shape->getVoxelResolution(),s.str().c_str());

            /*dout<<"mesh name = "<<s.str()<<endl;
            dout<<"maxCoord dim = "<<shape->getMaxDim()<<endl;
            dout<<"minCoord dim = "<<shape->getMinDim()<<endl;
            dout<<"Trans = "<<trans<<endl;
            dout<<"minCoord ("<<shape->getMin().x<<","<<shape->getMin().y<<","<<shape->getMin().z<<")"<<endl;
            dout<<"voxel res = "<<shape->getVoxelResolution()<<endl;
            dout<<"spacing = "<<space<<endl;*/

            return shape

}
    void GLWidget::display(bool blend)
    {
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
void GLWidget::setParameterValue(const QString& parameter, const string& value)
{

}
void GLWidget::loadScene(const QString& filename)
{
     scene->loadScene(filename);
     scene->loadMeshes(meshes,scene->getScene()->mRootNode);
     for(map<QString,Mesh*>::iterator i = meshs.begin(); i!=meshs.end(); i++)
     {
          ParticleShape* shape = createParticleShape(i->second)
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
          systems["rb1"]->addParticleShape(shape->getVoxelTexture(),shape->getMinDim(),shape->getMaxDim(),mat,shape->getVoxelResolution(),float4(0.0f,0.0f,0.0f,0.0f),float4(0.0f,0.0f,0.0f,1.0f),0.0f);
          pShapes[i->first]=shape;
     }
}
void GLWidget::loadMeshScene(const QString& filename)
{
     dynamicMeshScene->loadScene(filename);
     dynamicMeshScene->loadMeshes(dynamicMeshes,dynamicMeshScenes->getScene()->mRootNode);
     for(map<QString,Mesh*>::iterator i = meshs.begin(); i!=meshs.end(); i++)
     {
          ParticleShape* shape = createParticleShape(i->second)
          pShapes[i->first]=shape;
     }
}


void GLWidget::loadParameterFile(const QString& filename)
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
void GLWidget::ResetSimulations()
{

}
void GLWidget::parameterValueChanged(const QString& parameter, const QString& value)
{

}
