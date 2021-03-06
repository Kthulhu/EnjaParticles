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
        this->min=min*scale;
        this->max=max*scale;
        this->scale=scale;
        maxDim = this->max.x;
        if(maxDim<this->max.y)
            maxDim=this->max.y;
        if(maxDim<this->max.z)
            maxDim=this->max.z;
        minDim = this->min.x;
        if(minDim>this->min.y)
            minDim=this->min.y;
        if(minDim>this->min.z)
            minDim=this->min.z;

        delz=diameter;
        voxelResolution = ceil((maxDim-minDim)/diameter);
        glPushAttrib(GL_ENABLE_BIT|GL_TEXTURE_BIT);
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
        glTexImage3DEXT(GL_TEXTURE_3D_EXT, 0, GL_RGBA, voxelResolution, voxelResolution, voxelResolution, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glGenTextures(1, &surfaceTexture);
        glBindTexture(GL_TEXTURE_3D_EXT, surfaceTexture);
        /*glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_S, mode);
        glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_T, mode);
        glTexParameteri(GL_TEXTURE_3D_EXT, GL_TEXTURE_WRAP_R, mode);*/
        glTexImage3DEXT(GL_TEXTURE_3D_EXT, 0, GL_RGBA, voxelResolution, voxelResolution, voxelResolution, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        //glBindTexture(GL_TEXTURE_3D_EXT, 0);
        //glDisable(GL_TEXTURE_3D_EXT);
        glPopAttrib();
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

    float ParticleShape::getMinDim() const {
        return minDim;
    }

    int ParticleShape::getVoxelResolution() const {
        return voxelResolution;
    }
    ParticleShape::~ParticleShape(){
        glDeleteTextures(1,&volumeTexture);
        glDeleteTextures(1,&surfaceTexture);
    }
    void ParticleShape::voxelizeSurface(GLuint vbo, GLuint ibo, int length)
    {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
        glBindTexture(GL_TEXTURE_3D_EXT, surfaceTexture);
        GLuint fboId = 0;
        glGenFramebuffersEXT(1, &fboId);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,fboId);
        GLuint depth=0;
        glGenTextures(1, &depth);
        glBindTexture(GL_TEXTURE_2D, depth);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT32,voxelResolution,voxelResolution,0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,depth,0);
        //float col[4];
        //glGetFloatv(GL_COLOR_CLEAR_VALUE,col);
        glClearColor(0.0f,0.0f,0.0f,1.0f);
        //glEnable(GL_COLOR_LOGIC_OP);
        //glLogicOp(GL_OR);
        glDisable(GL_BLEND);
        glEnable(GL_POINT_SMOOTH);
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_MULTISAMPLE_EXT);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_CULL_FACE);
        glDisable(GL_LIGHTING);
        //int v[4];
        //glGetIntegerv(GL_VIEWPORT,v);
        glViewport(0,0,voxelResolution,voxelResolution);
        //glFramebufferTextureLayer( GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT , surfaceTexture, 0, 0 );
        glFramebufferTextureLayer( GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT , volumeTexture, 0, 0 );
        //glClear(GL_COLOR_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();

        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
        glReadBuffer(GL_COLOR_ATTACHMENT1_EXT);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexPointer(3, GL_FLOAT, 0, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glEnableClientState( GL_VERTEX_ARRAY );
        glDisable(GL_TEXTURE_3D_EXT);

        float mid=(maxDim+minDim)/2.0f;
        float half=mid-minDim;
        float curz=-half;
        glColor4f(1.0f,1.0f,1.0f,1.0f);
        for(int i = 0;i<voxelResolution; i++)
        {
            //glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, surfaceTexture, 0, i );
            glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, volumeTexture, 0, i );
            //glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            // set a 2D orthographic projection
            //glOrtho(minDim, maxDim, minDim, maxDim ,curz ,curz+delz);
            glOrtho(-half, half, -half, half ,curz ,curz+delz);
            curz+=delz;
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();

            glTranslatef(-(mid),-(mid),-(mid));
            glScalef(scale,scale,scale);
            glDrawElements(GL_TRIANGLES,length,GL_UNSIGNED_INT,0);

            glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
            glDrawElements(GL_TRIANGLES,length,GL_UNSIGNED_INT,0);
            glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
            glPopMatrix();
            glFlush();
        }
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
#if 0
        glDisableClientState( GL_VERTEX_ARRAY );
        glViewport(v[0],v[1],v[2],v[3]);
        glDisable(GL_POINT_SMOOTH);
        glDisable(GL_LINE_SMOOTH);
        glDisable(GL_MULTISAMPLE_EXT);
        glEnable(GL_DEPTH_TEST);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
        glDrawBuffer(GL_BACK);
        glClearColor(col[0],col[1],col[2],col[3]);
        glBindTexture(GL_TEXTURE_3D_EXT, 0);
        glDisable(GL_TEXTURE_3D_EXT);
#endif
        glPopAttrib();
        glPopClientAttrib();
    }
    void ParticleShape::voxelizeMesh(GLuint vbo, GLuint ibo, int length)
    {

        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
        glBindTexture(GL_TEXTURE_3D_EXT, volumeTexture);
        GLuint fboId = 0;
        glGenFramebuffersEXT(1, &fboId);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,fboId);
        GLuint depth=0;
        glGenTextures(1, &depth);
        glBindTexture(GL_TEXTURE_2D, depth);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT32,voxelResolution,voxelResolution,0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,depth,0);
        //float col[4];
        //glGetFloatv(GL_COLOR_CLEAR_VALUE,col);
        glClearColor(0.0f,0.0f,0.0f,1.0f);
        glEnable(GL_COLOR_LOGIC_OP);
        glLogicOp(GL_XOR);
        glDisable(GL_BLEND);//GL_DRAW_BUFFER0);
        //glBlendFunc(GL_ONE,GL_ONE);
        glDisable(GL_POINT_SMOOTH);
        glDisable(GL_LINE_SMOOTH);
        glDisable(GL_LIGHTING);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glShadeModel(GL_FLAT);
        //int v[4];
        //glGetIntegerv(GL_VIEWPORT,v);
        glViewport(0,0,voxelResolution,voxelResolution);
        glFramebufferTextureLayer( GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT , volumeTexture, 0, 0 );
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();

        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
        glReadBuffer(GL_COLOR_ATTACHMENT1_EXT);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexPointer(3, GL_FLOAT, 0, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glEnableClientState( GL_VERTEX_ARRAY );
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_TEXTURE_3D_EXT);
        float mid=(maxDim+minDim)/2.0f;
        float half=mid-minDim;
        float curz=-half;
        glColor4f(1.0f,1.0f,1.0f,1.0f);
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
            //glOrtho(-halfMaxDim, maxDim, -halfMaxDim, maxDim ,-halfMaxDim+delz*i ,-halfMaxDim+delz*(i+1));


            //glOrtho(min.x, min.x+maxDim, min.y, min.y+maxDim ,min.z+delz*i ,min.z+delz*(i+1));
            //glOrtho(minDim, maxDim, minDim, maxDim ,curz ,curz+delz);
            glOrtho(-half, half, -half, half ,curz ,curz+delz);
            //dout<<"MinDim = "<<minDim<<" MaxDim = "<<maxDim<<" curz = "<<curz<<" delz = "<<delz<<std::endl;
//            dout<<"-halfDim = "<<-halfMaxDim<<" halfMaxDim = "<<halfMaxDim<<std::endl;
            curz+=delz;
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glLoadIdentity();

            glTranslatef(-(mid),-(mid),-(mid));
            glScalef(scale,scale,scale);
            glDrawElements(GL_TRIANGLES,length,GL_UNSIGNED_INT,0);
            glPopMatrix();
            glFlush();
        }
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
        glPopAttrib();
        glPopClientAttrib();
        //voxelizeSurface(vbo,ibo,length);
        //glEnable(GL_COLOR_LOGIC_OP);
        //glEnable(GL_BLEND);//GL_DRAW_BUFFER0);
        //glLogicOp(GL_OR);

        //FIXME: I Need to render a full screen quad in order to have blending work correctly.
        /*for(int i = 0;i<voxelResolution; i++)
        {
            glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, volumeTexture, 0, i );
            //glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            glFramebufferTextureLayer(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, surfaceTexture, 0, i);
            glBlitFramebuffer(0,0,voxelResolution,voxelResolution,
                                0,0,voxelResolution,voxelResolution,
                                GL_COLOR_BUFFER_BIT,GL_NEAREST);
        }*/
#if 0
        glLogicOp(GL_COPY);
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_COLOR_LOGIC_OP);
		glDisableClientState( GL_VERTEX_ARRAY );
        glViewport(v[0],v[1],v[2],v[3]);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
        glDrawBuffer(GL_BACK);
        glClearColor(col[0],col[1],col[2],col[3]);
        glBindTexture(GL_TEXTURE_3D_EXT, 0);
        glShadeModel(GL_SMOOTH);
        glDisable(GL_TEXTURE_3D_EXT);
#endif
    }
}; //end namespace
