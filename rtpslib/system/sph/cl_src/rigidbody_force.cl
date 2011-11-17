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
#define ARGS __global float4* pos, __global float4* vel, __global float4* force, __global float4* pos_j, __global float4* vel_j, float stiffness, float dampening
#define ARGV pos, vel, force, pos_j, vel_j, stiffness, dampening

/*----------------------------------------------------------------------*/

#include "cl_macros.h"
#include "cl_structs.h"
//Contains all of the Smoothing Kernels for SPH
#include "cl_kernels.h"


//----------------------------------------------------------------------
inline void ForNeighbor(//__global float4*  vars_sorted,
                        ARGS,
                        PointData* pt,
                        uint index_i,
                        uint index_j,
                        float4 position_i,
                        __constant struct GridParams* gp,
                        __constant struct SPHParams* sphp
                        DEBUG_ARGS
                       )
{
    // get the particle info (in the current grid) to test against
    float4 position_j = pos_j[index_j] * sphp->simulation_scale; 
    float4 r = (position_j - position_i); 
    r.w = 0.f; // I stored density in 4th component
    // |r|
    float rlen = length(r);

    // is this particle within cutoff?
    if (rlen <= 2. * sphp->smoothing_distance)
    {

        //iej is 0 when we are looking at same particle
        //we allow calculations and just multiply force and xsph
        //by iej to avoid branching
        int iej = index_i != index_j;

        // avoid divide by 0 in Wspiky_dr
        rlen = max(rlen, sphp->EPSILON);

        float4 springForce = -stiffness*(2.*sphp->smoothing_distance-rlen)*(r/rlen); 

        float4 veli = vel[index_i]; // sorted
        float4 velj = vel_j[index_j];

        float4 dampeningForce = dampening*(velj-veli);
        //force *= sphp->mass;// * idi * idj;
        //FIXME: I think mass should be a part of one of these formulas. -ASY
        pt->force += (springForce+dampeningForce) * (float)iej;
        //pt->linear_force += r;//debug
    }
}

//Contains Iterate...Cells methods and ZeroPoint
#include "cl_neighbors.h"

//--------------------------------------------------------------
// compute forces on particles

__kernel void force_update(
                       //__global float4* vars_sorted,
                       ARGS,
                       __global int*    cell_indexes_start,
                       __global int*    cell_indexes_end,
                       __constant struct GridParams* gp,
                       __constant struct SPHParams* sphp 
                       DEBUG_ARGS
                       )
{
    // particle index
    int num = sphp->num;
    //int numParticles = get_global_size(0);
    //int num = get_global_size(0);

    int index = get_global_id(0);
    if (index >= num) return;

    float4 position_i = pos[index] * sphp->simulation_scale;

    //debuging
    clf[index] = (float4)(99,0,0,0);
    //cli[index].w = 0;

    // Do calculations on particles in neighboring cells
    PointData pt;
    zeroPoint(&pt);

    //IterateParticlesInNearbyCells(vars_sorted, &pt, num, index, position_i, cell_indexes_start, cell_indexes_end, gp,/* fp,*/ sphp DEBUG_ARGV);
    IterateParticlesInNearbyCells(ARGV, &pt, num, index, position_i, cell_indexes_start, cell_indexes_end, gp,/* fp,*/ sphp DEBUG_ARGV);
    force[index] += pt.force; 
    clf[index].xyz = pt.force.xyz;
}

/*-------------------------------------------------------------- */
#endif

