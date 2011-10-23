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

float magnitude(float4 vec)
{
    return sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
}       

__kernel void euler(
                   //__global float4* vars_unsorted, 
                   //__global float4* vars_sorted, 
                   //__global float4* positions,  // for VBO 
                   __global float4* pos_u, 
                   __global float4* pos_s, 
                   __global float4* vel_u, 
                   __global float4* vel_s, 
                   __global float4* linear_force_s, 
                   __global float4* torque_force_s, 
                   __global float4* color_u,
                   __global float4* color_s,
                   __global int* sort_indices,  
                   __constant struct ParticleRigidBodyParams* prbp, 
                   float dt)
{
    unsigned int i = get_global_id(0);
    int num = prbp->num;
    if (i >= num) return;

    float4 p = pos_s[i];
    float4 v = vel_s[i];
    float4 lf = linear_force_s[i];
    //float4 q = rot_s[i];
    //float4 w = avel_s[i];
    //float4 tf = torque_force_s[i];

    //external force is gravity
    //f.z += -9.8f;
    lf.z+=prbp.gravity;

    /*float speed = magnitude(f);
    if (speed > 600.0f) //velocity limit, need to pass in as struct
    {
        f *= 600.0f/speed;
    }
     */

    v += dt*lf;
    p += dt*v;
    p.w = 1.0f; //just in case
    p.xyz /= sphp->simulation_scale;
    //w += dt*tf;
    //q += dt*w;
    //q /= sphp->simulation_scale;

    uint originalIndex = sort_indices[i];

    vel_u[originalIndex] = v;
    //avel_u[originalIndex] = w;
    //unsorted_veleval(originalIndex) = v;
    //float dens = density(i);
    //unsorted_pos(originalIndex) = (float4)(p.xyz, dens);
    color_u[originalIndex] = color_s[i];
    pos_u[originalIndex] = (float4)(p.xyz, 1.);  // for plotting

}
