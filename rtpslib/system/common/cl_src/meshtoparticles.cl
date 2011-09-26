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
                            __constant int num,
                            __read_only image_3d_t posTex,
                            __constant float4 extent,
                            __constnat float4 min,
                            __constant float16 world,
                            __constant int res,
                            __global int newNum
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
    if(any(vox))
    {
        atom_inc(newNum);
        pos[num+newNum] = (float4)((s/(float)res)*extent.x+min.x,(t/(float)res)*extent.y+min.y,(r/(float)res)*extent.z+min.z,1.0);
        pos[num+newNum].x = dot(world.s0123,pos[num+newNum]);
        pos[num+newNum].x = dot(world.s4567,pos[num+newNum]);
        pos[num+newNum].x = dot(world.s89AB,pos[num+newNum]);
    }
}
//----------------------------------------------------------------------

#endif
