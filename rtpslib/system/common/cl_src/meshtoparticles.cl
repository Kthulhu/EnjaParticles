// This software contains source code provided by NVIDIA Corporation.
// Specifically code from the CUDA 2.3 SDK "Particles" sample

#ifndef _MESHTOPARTICLES_
#define _MESHTOPARTICLES_ 

#include "cl_macros.h"
#include "cl_structs.h"

#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_local_int32_base_atomics : enable

const sampler_t samp = CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;
//#pragma cl_khr_global_int32_base_atomics : enable
//----------------------------------------------------------------------
__kernel void meshToParticles(
                            __global float4* pos,
                            __global float4* color,
                            __global float4* velocity,
                            int num,
                            read_only image3d_t posTex,
                            float scale,
                            float4 min,
                            float16 world,
                            int res,
                            __global int* newNum
                            DEBUG_ARGS
                            )
{
    uint s = get_global_id(0);
    uint t = get_global_id(1);
    uint r = get_global_id(2);
    //int num = sphp->num;
    //int num = get_global_size(0);
    //if (index >= num) return;

    uint4 vox = read_imageui(posTex, samp, (int4)(s,t,r,0));
    //If voxel contains a one it's inside the mesh otherwise it can be ignored.
    if(vox.x>0)
    {
        int loc = atom_inc(newNum);
        pos[num+loc] = (float4)((s/(float)res)*scale+min.x,(t/(float)res)*scale+min.y,(r/(float)res)*scale+min.z,1.0);
        pos[num+loc].x = dot(world.s0123,pos[num+loc]);
        pos[num+loc].y = dot(world.s4567,pos[num+loc]);
        pos[num+loc].z = dot(world.s89AB,pos[num+loc]);
        color[num+loc] = (float4)(1.0,0.0,0.0,1.0);
        velocity[num+loc] = (float4)(0.0,0.0,0.0,0.0);
        clf[num+loc] = pos[num+loc];
    }
}
//----------------------------------------------------------------------

#endif
