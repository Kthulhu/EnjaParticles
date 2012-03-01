
#ifndef _SAMPLE_
#define _SAMPLE_

#include "cl_structs.h"

//#pragma cl_khr_global_int32_base_atomics : enable
//----------------------------------------------------------------------
__kernel void sample(int popSize,
                            __global float4* population,
                            int sampSize,
                            __global float4* samples,
                            __global uint* indices,
                            uint insOffset, 
                            uint insStride//,
                            //__global float4* debugf,
                            //__global int4* debugi
                            )
{
    uint index = get_global_id(0);
    if (index >= sampSize) return;
    uint sample_index = indices[index];
    samples[(index*insStride)+insOffset] = (float)(sample_index<popSize)* population[sample_index];
    //debugi[index]=(int4)(index,sample_index,(index*insStride),(index*insStride)+insOffset);
    //debugf[index]=(float)(sample_index<popSize)* population[sample_index];
}
//----------------------------------------------------------------------

#endif
