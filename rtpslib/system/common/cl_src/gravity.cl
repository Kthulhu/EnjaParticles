
#ifndef _GRAVITY_
#define _GRAVITY_

#include "cl_structs.h"

//#pragma cl_khr_global_int32_base_atomics : enable
//----------------------------------------------------------------------
__kernel void gravity(int numPart,
                      int numPoint,
                            __global float4* pointSources,
                            __global float* massSources,
                            float alpha,
                            __global float4* pos,
                            __global float4* accel
                            )
{
    uint index = get_global_id(0);
    if (index >= num) return;
    for(int i = 0; i<numPoint; i++)
    {
        float4 direction = normalize(pointSources[i]-pos[index])
        float mag = alpha;
        accel += mag*direction;        
    }
}
//----------------------------------------------------------------------

#endif
