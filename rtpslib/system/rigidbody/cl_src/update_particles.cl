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


//These are passed along through cl_neighbors.h
//only used inside ForNeighbor defined in this file
#define ARGS __global float4* pos, __global float4* vel, __global float4* linear_force
//, __global float4* torque_force
#define ARGV pos, vel, linear_force 

/*----------------------------------------------------------------------*/

#include "cl_macros.h"
#include "cl_structs.h"
#include "Quaternion.h"
/*__inline__ float16 quatToRot(float4 quat)
{
    float a2 = quat.x*quat.x;
    float b2 = quat.y*quat.y;
    float c2 = quat.z*quat.z;
    float d2 = quat.w*quat.w;
    float bc = quat.y*quat.z;
    float ad = quat.x*quat.w;
    float bd = quat.y*quat.w;
    float ac = quat.x*quat.z;
    float ab = quat.x*quat.y;
    float cd = quat.z*quat.z;
    return (float16)(a2+b2-c2-d2,2.0*(bc-ad),2.0*(bd+ac),0.0,
                        2.0*(bc+ad),a2-b2+c2-d2,2.0*(cd-ab), 0.0,
                        2.0*(bd-ac),2.0*(cd+ab),a2-b2-c2+d2, 0.0,
                        0.0, 0.0, 0.0, 1.0);
}*/
__kernel void update_particles(
                    __global float4* pos_u,
                    __global float4* pos_l,
                    __global float4* velocity_u,
                    __global int2* particleIndex,
                    __global float4* comPos,
                    __global float4* comRot,
                    __global float4* comVel,
                    __global float4* comAngVel
                       DEBUG_ARGS
                       )
{
    int index = get_global_id(0);
    int i = particleIndex[index].x;
    int end = particleIndex[index].y;
    float16 m = qtGetRotationMatrix(comRot[index]); 
    for(;i<end;i++)
    {
        float3 rotPos =(float3)(dot(m.s012,pos_l[i].xyz),dot(m.s456,pos_l[i].xyz),dot(m.s89a,pos_l[i].xyz));
        //pos_u[i]=rotPos+comPos[index];
        pos_u[i].xyz=rotPos+comPos[index].xyz;
        pos_u[i].w = 1.0;
        velocity_u[i].xyz = comVel[index].xyz + cross(comAngVel[index].xyz,pos_l[i].xyz);
        //clf[i]=pos_l[i];
        //clf[i]=pos_u[i];
        //clf[i].xyz=rotPos;
        //clf[i].xyz=cross(comAngVel[index].xyz,pos_l[i].xyz);
        //clf[i].xyz=comAngVel[index].xyz;
    }
    clf[index]=comRot[index];
    //clf[index*4]=m.s0123;
    //clf[index*4+1]=m.s4567;
    //clf[index*4+2]=m.s89ab;
    //clf[index*4+3]=m.scdef;
}
/*-------------------------------------------------------------- */
#endif

