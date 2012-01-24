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
#include <math.h>
#include <sstream>
#include <iomanip>
#include <string>
#include <limits.h>

#include "ParticleShape.h"
#include "../render/RenderUtils.h"
namespace rtps
{
    ParticleShape::ParticleShape(float3 min, float3 max, float diameter, float scale){
        dim = (max-min)*scale;
        maxDim = dim.x;
        if(maxDim<dim.y)
            maxDim=dim.y;
        if(maxDim<dim.z)
            maxDim=dim.z;
        this->min=min*scale;
        this->max=max*scale;
        this->scale=scale;
        delz=diameter;
        voxelResolution = maxDim/diameter;
        
        printf("3d texture supported? %d\n",glewIsSupported("GL_EXT_texture3D"));
        glEnable(GL_TEXTURE_3D_EXT);
        glGenTextures(1, &volumeTexture);
        glBindTexture(GL_TEXTURE_3D_EXT, volumeTexture);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        GLint mode = GL_CLAMP_TO_BORDER;
        glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_S, mode);
        glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_T, mode);
        glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_R, mode);
        //img is for debugging
        //unsigned char img[voxelResolution*voxelResolution*voxelResolution*4];
        //memset(img,0,sizeof(unsigned char)*voxelResolution*voxelResolution*voxelResolution*4);
        //glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage3DEXT(GL_TEXTURE_3D_EXT, 0, GL_RGBA, voxelResolution, voxelResolution, voxelResolution, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glBindTexture(GL_TEXTURE_3D_EXT, 0);
        glDisable(GL_TEXTURE_3D_EXT);
    }

    float3 ParticleShape::getMax() const {
        return max;
    }

    float3 ParticleShape::getMin() const {
        return min;
    }

    float3 ParticleShape::getDim() const {
        return dim;
    }

    float ParticleShape::getMaxDim() const {
        return maxDim;
    }

    int ParticleShape::getVoxelResolution() const {
        return voxelResolution;
    }
    ParticleShape::~ParticleShape(){
        glDeleteTextures(1,&volumeTexture);
    }
    void ParticleShape::voxelizeSurface(GLuint vbo, GLuint ibo, int length)
    {
        
    }
    void ParticleShape::voxelizeMesh(GLuint vbo, GLuint ibo, int length)
    {
        
        glEnable(GL_TEXTURE_3D_EXT);
        glBindTexture(GL_TEXTURE_3D_EXT, volumeTexture);
        GLuint fboId = 0;
        glGenFramebuffersEXT(1, &fboId);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,fboId);
        //glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT,fboId);
        //glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT,fboId);
        //glFramebufferTexture3DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT , GL_TEXTURE_3D_EXT, volumeTexture, 0, 0 );
        GLuint depth=0;
        glGenTextures(1, &depth);
        glBindTexture(GL_TEXTURE_2D, depth);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT32,voxelResolution,voxelResolution,0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,depth,0);
        float col[4];
        glGetFloatv(GL_COLOR_CLEAR_VALUE,col);
        glClearColor(0.0f,0.0f,0.0f,0.0f);
        //FIXME: Code should check and preserve the current state so that
        //It correctly restores previous state.
        /*/glEnable(GL_COLOR_LOGIC_OP);
        glEnable(GL_BLEND);//GL_DRAW_BUFFER0);
        glBlendFunc(GL_SRC_COLOR,GL_DST_COLOR);
        glLogicOp(GL_XOR);
        glBlendFunc(GL_ONE,GL_ONE);
        glDisable(GL_ALPHA_TEST);
        glDisable(GL_POINT_SMOOTH);
        glDisable(GL_LINE_SMOOTH);
        glDisable(GL_LIGHTING);*/
        //glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        //glDisable(GL_TEXTURE_2D);
        int v[4];
        glGetIntegerv(GL_VIEWPORT,v);
        glViewport(0,0,voxelResolution,voxelResolution);
        glFramebufferTextureLayer( GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT , volumeTexture, 0, 0 );
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_TEXTURE_2D);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
        glReadBuffer(GL_COLOR_ATTACHMENT1_EXT);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexPointer(3, GL_FLOAT, 0, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glEnableClientState( GL_VERTEX_ARRAY );
        float halfMaxDim=maxDim/2.0f;
        for(int i = 0;i<voxelResolution; i++)
        {
            if(i>0)
            {
                glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, volumeTexture, 0, i );
                glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
                glFramebufferTextureLayer(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, volumeTexture, 0, i-1);
                glBlitFramebuffer(0,0,voxelResolution,voxelResolution,
                                    0,0,voxelResolution,voxelResolution,
                                    GL_COLOR_BUFFER_BIT,GL_NEAREST);
            }
            // switch to projection mode
            glMatrixMode(GL_PROJECTION);
            // save previous matrix which contains the 
            //settings for the perspective projection
            // reset matrix
            glLoadIdentity();
            // set a 2D orthographic projection
            glOrtho(-halfMaxDim, halfMaxDim, -halfMaxDim, halfMaxDim ,-halfMaxDim+delz*i ,-halfMaxDim+delz*(i+1));
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();

            glScalef(scale,scale,scale);
            glColor4f(1.0f,0.0f,1.0f,1.0f);
            glDrawElements(GL_TRIANGLES,length,GL_UNSIGNED_INT,0); 
            //glBegin(GL_TRIANGLES);
            //glVertex3f(-halfMaxDim,-halfMaxDim,-halfMaxDim);
            //glVertex3f(halfMaxDim,halfMaxDim,halfMaxDim);
            //glVertex3f(halfMaxDim,-halfMaxDim,-halfMaxDim);
            //glVertex3f(0.0f,0.0f,0.0f);//(-halfMaxDim+delz*i-halfMaxDim+delz*(i+1))/2.0f);
            //glVertex3f(0.0f,.5f,-0.5f);//(-halfMaxDim+delz*i-halfMaxDim+delz*(i+1))/2.0f);
            //glVertex3f(.5f,0.f,-0.5f);//(-halfMaxDim+delz*i-halfMaxDim+delz*(i+1))/2.0f);
            //glEnd();
            glPopMatrix();
            glFlush();
        }
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glDisableClientState( GL_VERTEX_ARRAY );
        glViewport(v[0],v[1],v[2],v[3]);
        glEnable(GL_DEPTH_TEST);
        /*glEnable(GL_ALPHA_TEST);
        glEnable(GL_TEXTURE_2D);
        //glEnable(GL_CULL_FACE);
        //glEnable(GL_LIGHTING);
        glDisable(GL_BLEND);
        glLogicOp(GL_COPY);
        glDisable(GL_COLOR_LOGIC_OP);*/
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
        glClearColor(col[0],col[1],col[2],col[3]);
        glMatrixMode(GL_MODELVIEW);
        glBindTexture(GL_TEXTURE_3D_EXT, 0);
        glDisable(GL_TEXTURE_3D_EXT);
    }
}; //end namespace
