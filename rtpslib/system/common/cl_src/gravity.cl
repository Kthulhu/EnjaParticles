
#ifndef _GRAVITY_
#define _GRAVITY_

#include "cl_structs.h"

//#pragma cl_khr_global_int32_base_atomics : enable
//----------------------------------------------------------------------
__kernel void gravity(int numPart,
                      int numPoint,
                            __global float4* pointSources,
                            __global float* massSources,
                            __global float* alpha,
                            __global float4* pos,
                            __global float4* accel,
                            float scale
                            )
{
    uint index = get_global_id(0);
    if (index >= numPart) return;
    for(int i = 0; i<numPoint; i++)
    {
        float4 vect = scale*(pointSources[i]-pos[index]);
        float3 direction = normalize(vect.xyz);
        float dist2 = dot(vect.xyz,vect.xyz);
        float mag = alpha[i]*massSources[i];
        float cutoff = 0.5*scale;
        cutoff*=cutoff;
        if(dist2<cutoff)
            mag/=-cutoff;
        else
            mag/=dist2;
        accel[index] += (float4)((mag*direction),0.0f);        
    }
}
//----------------------------------------------------------------------
//----------------------------------------------------------------------
__kernel void gravityForce(int numPart,
                      int numPoint,
                            __global float4* pointSources,
                            __global float* massSources,
                            __global float* alpha,
                            __global float4* pos,
                            __global float* mass,
                            __global float4* force,
                            float scale
                            )
{
    uint index = get_global_id(0);
    if (index >= numPart) return;
    for(int i = 0; i<numPoint; i++)
    {
        float4 vect = scale*(pointSources[i]-pos[index]);
        float3 direction = normalize(vect.xyz);
        float dist2 = dot(vect.xyz,vect.xyz);
        float mag = alpha[i]*massSources[i];
        float cutoff = 0.5*scale;
        cutoff*=cutoff;
        if(dist2<cutoff)
            mag/=-cutoff;
        else
            mag/=dist2;
        force[index] += ((float4)((mag*direction),0.0f))*mass[index];        
    }
}

#endif
