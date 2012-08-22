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


#include "cl_PRB_macros.h"
#include "cl_PRB_structs.h"
#include "Quaternion.h"

float magnitude(float4 vec)
{
    return sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
}       

__kernel void euler(
                   __global float4* comLinearForce,
                   __global float4* comTorqueForce,
                   __global float4* comVel,
                   __global float4* comAngVel,
                   __global float4* comAngMomentum,
                   __global float4* comPos,
                   __global float4* comRot, 
                   __global float16* inertialTensor, 
                   __global float* rbMass, 
                    float4 gravity,
                   float dt,
                    __constant struct ParticleRigidBodyParams* prbp
                    DEBUG_ARGS)
{
    unsigned int i = get_global_id(0);

    float4 p = comPos[i]*prbp[0].simulation_scale ;
    float4 v = comVel[i];
    float4 lf = comLinearForce[i];
    float4 L = comAngMomentum[i]; 
    Quaternion q = comRot[i];
    float4 w = comAngVel[i];
    float4 tf = comTorqueForce[i];

    //external force is gravity
    //f.z += -9.8f;
    lf+=rbMass[i]*gravity;

    float4 a = (lf/rbMass[i]);
    float speed = magnitude(a);
    if (speed > prbp[0].velocity_limit) //velocity limit, need to pass in as struct
    {
        a *= prbp[0].velocity_limit/speed;
    }
     

    v += dt*(a);
    p += dt*v;
    p.xyz/=prbp[0].simulation_scale;
    p.w = 1.0f; //just in case
    L += dt*(tf);
    L.w = 0.0f;
    w.x= dot(inertialTensor[i].s0123,L);
    w.y= dot(inertialTensor[i].s4567,L);
    w.z= dot(inertialTensor[i].s89ab,L);
    w.w = 0.0f;
    Quaternion dq = qtSet(w,sqrt(dot(dt*w,dt*w)));
    q = qtMul(dq,q);
    comVel[i] = v;
    comPos[i] = p;
    comAngVel[i] = w;
    comAngMomentum[i] = L;
    comRot[i] = q;
    
    clf[i] = tf;
}
