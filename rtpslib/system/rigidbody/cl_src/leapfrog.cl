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



#include "cl_macros.h"
#include "cl_structs.h"
#include "Quaternion.h"

__kernel void leapfrog(
                   __global float4* comLinearForce,
                   __global float4* comTorqueForce,
                   __global float4* comVel,
                   __global float4* comAngVel,
                   __global float4* comVelEval,
                   __global float4* comAngVelEval,
                   __global float4* comPos,
                   __global float4* comRot, 
                   __global float16* inertialTensor, 
                   __global float* rbMass, 
                   float dt,
                    __constant struct ParticleRigidBodyParams* prbp
                    DEBUG_ARGS)
{
    unsigned int i = get_global_id(0);
    float4 p = comPos[i]*prbp->simulation_scale ;
    float4 v = comVel[i];
    float4 lf = comLinearForce[i];
    Quaternion q = comRot[i];
    float4 w = comAngVel[i];
    float4 tf = comTorqueForce[i];

    lf+=rbMass[i]*prbp->gravity;

    float4 a = lf/rbMass[i];
    a.w=0.0f;
    float speed = length(a);
    if (speed > 600.0f) //velocity limit, need to pass in as struct
    {
        a *= 600.0f/speed;
    }

    float4 vnext = v + dt*a;

    float4 veval = 0.5f*(v+vnext);
    p += dt*vnext;
    p.xyz/=prbp->simulation_scale;
    p.w = 1.0f; //just in case
    //need to fix torque scaling.
    float4 L = dt*(tf);
    L.w = 0.0f;
    float4 wnext = w; 
    wnext.x+= dot(inertialTensor[i].s0123,L);
    wnext.y+= dot(inertialTensor[i].s4567,L);
    wnext.z+= dot(inertialTensor[i].s89ab,L);
    wnext.w = 0.0f;
    float4 weval=0.5f*(w+wnext);
    Quaternion dq = qtSet(w,sqrt(dot(dt*wnext,dt*wnext)));
    q = qtMul(dq,q);
    comVel[i] = vnext;
    comVelEval[i] = veval;
    comPos[i] = p;
    comAngVel[i] = wnext;
    comAngVelEval[i] = weval;
    comRot[i] = q;
    //clf[i]=a;
    //clf[i]=vnext;
    clf[i]=v;
}
