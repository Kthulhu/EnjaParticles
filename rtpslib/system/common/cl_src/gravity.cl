
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
        vect.w=0.0f;
        float4 direction = fast_normalize(vect);
        float dist2 = dot(vect,vect);
        float mag = alpha[i]*massSources[i];
        float cutoff = 0.5f*scale;
        cutoff*=cutoff;
        if(dist2<cutoff)
            mag/=-cutoff;
        else
            mag/=dist2;
        accel[index] += (mag*direction);
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
        vect.w=0.0f;
        float4 direction = fast_normalize(vect);
        float dist2 = dot(vect,vect);
        float mag = alpha[i]*massSources[i];
        float cutoff = 0.5f*scale;
        cutoff*=cutoff;
        if(dist2<cutoff)
            mag/=-cutoff;
        else
            mag/=dist2;
        force[index] += (mag*direction.xyzw)*mass[index];
        //force[index]+=(mag * (float4)(1.0f,1.0f,1.0f,0.0f))*mass[index];
        //force[index]+=direction;
    }
}

#endif
