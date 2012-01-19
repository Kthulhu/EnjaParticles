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
namespace rtps
{
    ParticleShape::ParticleShape(float3 min, float3 max, float diameter){
        float3 dim = max-min;
        float maxDim = dim.x;
        if(maxDim<dim.y)
            maxDim=dim.y;
        if(maxDim<dim.z)
            maxDim=dim.z;
        this->dim=dim;
        this->min=min;
        this->max=max;
        delz=diameter;
        voxelResolution = maxDim/diameter;
        
        printf("3d texture supported? %d\n",glewIsSupported("GL_EXT_texture3D"));
        glEnable(GL_TEXTURE_3D_EXT);
        //glEnable(GL_DRAW_BUFFER);
        //glEnable(GL_FRAMEBUFFER);
        GLuint volumeTexture=0;
        glGenTextures(1, &volumeTexture);
        printf("volumeTexture = %d\n",volumeTexture);
        printf("voxelResolution = %d\n",voxelResolution);
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
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage3DEXT(GL_TEXTURE_3D_EXT, 0, GL_RGBA, voxelResolution, voxelResolution, voxelResolution, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
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
        
        //img);
        GLuint fboId = 0;
        glGenFramebuffersEXT(1, &fboId);
        glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT,fboId);
        //glFramebufferTexture3DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT , GL_TEXTURE_3D_EXT, volumeTexture, 0, 0 );
        float col[4];
        glGetFloatv(GL_COLOR_CLEAR_VALUE,col);
        glClearColor(0.0f,0.0f,0.0f,1.0f);
        int i = 0;
        //FIXME: Code should check and preserve the current state so that
        //It correctly restores previous state.
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE,GL_ONE);
        glLogicOp(GL_XOR);
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        int v[4];
        glGetIntegerv(GL_VIEWPORT,v);
        glViewport(0,0,voxelResolution,voxelResolution);
        glFramebufferTextureLayer( GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT , volumeTexture, 0, i );

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexPointer(3, GL_FLOAT, 0, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glEnableClientState( GL_VERTEX_ARRAY );
        for(int i = 0; i<voxelResolution; i++)
        {
            glFramebufferTextureLayer( GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT , volumeTexture, 0, i );
            glDrawBuffer(GL_AUX0);
            glClear(GL_COLOR_BUFFER_BIT);
            glFramebufferTextureLayer(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, volumeTexture, 0, i?i-1:0);
            glReadBuffer(GL_AUX1);
            // switch to projection mode
            glMatrixMode(GL_PROJECTION);
            // save previous matrix which contains the 
            //settings for the perspective projection
            glPushMatrix();
            // reset matrix
            glLoadIdentity();
            // set a 2D orthographic projection
            //glOrtho(0, maxDim, 0, maxDim,delz*i*maxDim,delz*(i+1)*maxDim);
            glOrtho(-maxDim, maxDim, -maxDim, maxDim ,-maxDim+delz*i ,-maxDim+delz*(i+1));
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();

            glTranslatef(0.f,0.f,maxDim);
            glDrawElements(GL_TRIANGLES,length,GL_UNSIGNED_INT,0); 
            glPopMatrix();
            glMatrixMode(GL_PROJECTION);
            glPopMatrix();
        }
        glDisableClientState( GL_VERTEX_ARRAY );
        glViewport(0,0,v[3],v[4]);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        glDisable(GL_BLEND);
        glLogicOp(GL_COPY);
        glEnable(GL_CULL_FACE);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
        glClearColor(col[0],col[1],col[2],col[3]);
        glMatrixMode(GL_MODELVIEW);
        glDisable(GL_TEXTURE_3D_EXT);
    }
}; //end namespace
