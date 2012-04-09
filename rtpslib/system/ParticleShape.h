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


#ifndef RTPS_PARTICLE_SHAPE_H_INCLUDED
#define RTPS_PARTICLE_SHAPE_H_INCLUDED

#include "../structs.h"
#include "../timer_eb.h"
#include "../rtps_common.h"

namespace rtps
{
    class RTPS_EXPORT ParticleShape
    {
    public:
       ParticleShape(float3 min, float3 max, float diameter, float scale=1.0f);
       ~ParticleShape();
       void voxelizeMesh(GLuint vbo, GLuint ibo, int length);
       void voxelizeSurface(GLuint vbo, GLuint ibo, int length);
       GLuint getVoxelTexture()
       {
           return volumeTexture;
       }
       GLuint getSurfaceTexture()
       {
           return surfaceTexture;
       }
       float3 getMax() const;
       float3 getMin() const;
       float3 getDim() const;
       float getMaxDim() const;
       float getMinDim() const;
       int getVoxelResolution() const;

    protected:
        GLuint volumeTexture;
        GLuint surfaceTexture;
        int voxelResolution;
        float maxDim,minDim,scale,delz;
        float3 dim,min,max;
    };

}

#endif
