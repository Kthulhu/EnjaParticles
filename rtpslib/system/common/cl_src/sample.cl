
#ifndef _SAMPLE_
#define _SAMPLE_

#include "cl_structs.h"

//#pragma cl_khr_global_int32_base_atomics : enable
//----------------------------------------------------------------------
__kernel void sample(int num,
                            __global float4* population,
                            __global float4* samples,
                            __global uint* indices,
                            uint insOffset, 
                            uint insStride
                            )
{
    uint index = get_global_id(0);
    if (index >= num) return;
    uint sample_index = indices[index];
    samples[(index*insStride)+insOffset] = population[sample_index];
}
//----------------------------------------------------------------------

#endif
