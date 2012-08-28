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

float16 MultiplyMatrix3x3(const float16& A,const float16& B)
{
    float16 ret;
    ret.s0=dot(A.s012,B.s048);
    ret.s1=dot(A.s012,B.s159);
    ret.s2=dot(A.s012,B.s26a);
    ret.s3=0.0f;
    ret.s4=dot(A.s456,B.s048);
    ret.s5=dot(A.s456,B.s159);
    ret.s6=dot(A.s456,B.s26a);
    ret.s7=0.0f;
    ret.s8=dot(A.s89a,B.s048);
    ret.s9=dot(A.s89a,B.s159);
    ret.sa=dot(A.s89a,B.s26a);
    ret.sb=0.0f;
    ret.sc=0.0f;
    ret.sd=0.0f;
    ret.se=0.0f;
    ret.sf=0.0f;
}

__kernel void leapfrog(
                   __global float4* comLinearForce,
                   __global float4* comTorqueForce,
                   __global float4* comVel,
                   __global float4* comAngVel,
                   __global float4* comAngMomentum,
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
    float4 p = comPos[i]*prbp[0].simulation_scale ;
    float4 v = comVel[i];
    float4 lf = comLinearForce[i];
    Quaternion q = comRot[i];
    float4 w = comAngVel[i];
    float4 tf = comTorqueForce[i];
    float4 L = comAngMomentum[i];

    lf+=rbMass[i]*prbp[0].gravity;

    float4 a = lf/rbMass[i];
    a.w=0.0f;
    float speed = length(a);
    if (speed >prbp[0].velocity_limit ) //velocity limit, need to pass in as struct
    {
        a *= prbp[0].velocity_limit/speed;
    }

    float4 vnext = v + dt*a;

    float4 veval = 0.5f*(v+vnext);
    p += dt*vnext;
    p.xyz/=prbp[0].simulation_scale;
    p.w = 1.0f; //just in case
    //need to fix torque scaling.

    float4 Lnext = L+dt*(tf);
    L = 0.5*(L+Lnext);
    //float4 wnext = w; 
    //wnext.x+= dot(inertialTensor[i].s0123,L);
    //wnext.y+= dot(inertialTensor[i].s4567,L);
    //wnext.z+= dot(inertialTensor[i].s89ab,L);
    w.x= dot(inertialTensor[i].s012,Lnext);
    w.y= dot(inertialTensor[i].s456,Lnext);
    w.z= dot(inertialTensor[i].s89a,Lnext);
    w.w = 0.0f;
    Quaternion dq = qtSet(w,sqrt(dot(dt*w,dt*w)));
    //Quaternion dq = qtSet(wnext,sqrt(dot(dt*wnext,dt*wnext)));
    q = qtMul(dq,q);
    comVel[i] = vnext;
    comVelEval[i] = veval;
    comPos[i] = p;
    comAngVel[i] = w;
    comAngMomentum[i] = L;
    //comAngVelEval[i] = weval;
    //normalize q to prevent instablity due to numerical round off.
    comRot[i] = q/length(q);
    
    //now we need to update the inverse inertial tensor.
    float16 rotMatrix = qtGetRotationMatrix(q);
    float16 rotMatrixT = transpose(rotMatrix);
    inertialTensor[i] = MultiplyMatrix3x3(rotMatrix,MultiplyMatrix3x3(inertialTensor[i],rotMatrixT));
}
