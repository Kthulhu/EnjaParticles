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


#ifndef _SEGMENTED_SCAN_CL_
#define _SEGMENTED_SCAN_CL_

#include "cl_PRB_macros.h"
#include "cl_PRB_structs.h"
#include "Quaternion.h"
float4 rot(float4 v, Quaternion q)
{
    float4 vtemp = qtMul(q,v);
    q.xyz = -q.xyz;
    return qtMul(vtemp,q);
}
__kernel void sum(
                    __global float4* pos_l,
                    __global int2* particleIndex,
                    __global float4* linear_force_s,
                    __global float4* comLinearForce,
                    __global float4* comTorqueForce,
                    __global float4* comPos,
                    __global float4* comRot,
                    __constant struct ParticleRigidBodyParams* prbp
                       DEBUG_ARGS
                       )
{
    int index = get_global_id(0);
    int i = particleIndex[index].x;
    int end = particleIndex[index].y;
    comLinearForce[index]=linear_force_s[i];
    comTorqueForce[index].xyz=cross3F4(rot(pos_l[i],comRot[i]),linear_force_s[i]).xyz;
    i++;
    for(;i<end;i++)
    {
        comLinearForce[index]+=linear_force_s[i];
	    comTorqueForce[index].xyz+=cross3F4(rot(pos_l[i],comRot[i]),linear_force_s[i]).xyz;
        //clf[i].xyz=pos_u[i].xyz;
        //clf[i].xyz=(pos_u[i]-comPos[index]).xyz;
    }
    //clf[index]=comTorqueForce[index];
    clf[index]=comLinearForce[index];
}
#endif

