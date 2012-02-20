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


#ifndef _NEIGHBORS_CL_
#define _NEIGHBORS_CL_


//These are passed along through cl_neighbors.h
//only used inside ForNeighbor defined in this file
#define ARGS __global float4* pos, __global float4* vel, __global float4* linear_force, __global float* mass, __global float4* pos_j
#define ARGV pos, vel, linear_force, mass, pos_j

/*----------------------------------------------------------------------*/

#include "cl_macros.h"
#include "cl_structs.h"
//Contains all of the Smoothing Kernels for SPH
//#include "cl_kernels.h"


//----------------------------------------------------------------------
inline void ForNeighbor(//__global float4*  vars_sorted,
                        ARGS,
                        PointData* pt,
                        uint index_i,
                        uint index_j,
                        float4 position_i,
                        __constant struct GridParams* gp,
                        __constant struct ParticleRigidBodyParams* prbp
                        DEBUG_ARGS
                       )
{
    //int num = prbp->num;

    // get the particle info (in the current grid) to test against
    float4 position_j = pos_j[index_j] * prbp->simulation_scale;
    float4 r = (position_j - position_i);
    r.w = 0.f; // I stored density in 4th component
    // |r|
    float rlen = length(r);

    float4 norm = r/rlen;
    // is this particle within cutoff?
    if (rlen <= 2.*prbp->smoothing_distance)
    {
        rlen = max(rlen, prbp->EPSILON);
        float massnorm = ((mass[index_i]*mass[index_i])/(mass[index_i]+mass[index_i]));
        float stiff = (prbp->spring*massnorm);
        float4 springForce = -stiff*(2.*prbp->smoothing_distance-rlen)*(norm);

        float4 relvel = -vel[index_i];

        float4 dampeningForce = prbp->dampening*sqrt(stiff*massnorm)*(relvel);
        float4 normalForce=(springForce+dot(dampeningForce,norm)*norm); 

        relvel.w=0.0;
        normalForce.w=0.0;
        //Use Gram Schmidt process to find tangential velocity to the particle
        float4 tangVel=relvel-dot(relvel,norm)*norm;
        float4 frictionalForce=0.0f;
        if(length(tangVel)>prbp->friction_static_threshold)
            frictionalForce = -prbp->friction_dynamic*length(normalForce)*(normalize(tangVel));
        else
            frictionalForce = -prbp->friction_static*tangVel;
        
        pt->linear_force += (springForce+dampeningForce+frictionalForce);
    }
}

//Contains Iterate...Cells methods and ZeroPoint
#include "cl_neighbors.h"

//--------------------------------------------------------------
// compute forces on particles

__kernel void force_update(
                       //__global float4* vars_sorted,
                       ARGS,
                       __global int* sort_indices,
                       __global int*    cell_indexes_start,
                       __global int*    cell_indexes_end,
                       __constant struct GridParams* gp,
                       __constant struct ParticleRigidBodyParams* prbp
                       DEBUG_ARGS
                       )
{
    // particle index
    int num = prbp->num;
    int index = get_global_id(0);
    if (index >= num) return;

    float4 position_i = pos[index] * prbp->simulation_scale;

    // Do calculations on particles in neighboring cells
    PointData pt;
    zeroPoint(&pt);

    IterateParticlesInNearbyCells(ARGV, &pt, num, index, position_i, cell_indexes_start, cell_indexes_end, gp,/* fp,*/ prbp DEBUG_ARGV);

    linear_force[sort_indices[index]] += pt.linear_force;
    clf[sort_indices[index]] = pt.linear_force;
}

/*-------------------------------------------------------------- */
#endif

