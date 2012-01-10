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


#ifndef _UPDATE_PARTICLES_CL_
#define _UPDATE_PARTICLES_CL_


/*----------------------------------------------------------------------*/

#include "cl_macros.h"
#include "cl_structs.h"
#include "Quaternion.h"
__kernel void update_particles(
                    __global float4* pos_u,
                    __global float4* pos_l,
                    __global float4* velocity_u,
                    __global int2* particleIndex,
                    __global float4* comPos,
                    __global float4* comRot,
                    __global float4* comVel,
                    __global float4* comAngVel,
                     __constant struct ParticleRigidBodyParams* prbp
                       DEBUG_ARGS
                       )
{
    int index = get_global_id(0);
    int i = particleIndex[index].x;
    int end = particleIndex[index].y;
    float16 m = qtGetRotationMatrix(comRot[index]); 
    for(;i<end;i++)
    {
        float4 scalePos=pos_l[i]*prbp->simulation_scale;
        float4 rotPos =(float4)(dot(m.s0123,scalePos),dot(m.s4567,scalePos),dot(m.s89ab,pos_l[i]*scalePos),0.0f);
        //pos_u[i]=rotPos+comPos[index];
        pos_u[i].xyz=(rotPos.xyz/prbp->simulation_scale)+comPos[index].xyz;
        pos_u[i].w = 1.0;
        velocity_u[i].xyz = comVel[index].xyz + cross3F4(comAngVel[index],scalePos).xyz;
        clf[i]=velocity_u[i];
        //clf[i]=pos_u[i];
        //clf[i].xyz=rotPos;
        //clf[i].xyz=cross(comAngVel[index].xyz,pos_l[i].xyz);
        //clf[i].xyz=comAngVel[index].xyz;
    }
    //clf[index]=comRot[index];
    //clf[index*4]=m.s0123;
    //clf[index*4+1]=m.s4567;
    //clf[index*4+2]=m.s89ab;
    //clf[index*4+3]=m.scdef;
}
/*-------------------------------------------------------------- */
#endif

