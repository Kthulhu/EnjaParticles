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
#define ARGS __global float4* pos, __global float* density, global float4* img
#define ARGV pos, density, img

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
    float4 position_j = pos[index_j] * sphp->simulation_scale;
    float4 r = (position_i - position_j);
    r.w = 0.f; // I stored density in 4th component
    // |r|
    float rlen = length(r);

    // is this particle within cutoff?
    if (rlen <= sphp->smoothing_distance)
    {
        // avoid divide by 0 in Wspiky_dr
        rlen = max(rlen, sphp->EPSILON);

        float dWijdr = Wspiky(rlen, sphp->smoothing_distance, sphp);
        float dj = density[index_j];
        float idj = 1.0/dj;

        float kern = dWijdr* idj;

        pt->force.x += kern;
        //debugging
        pt->force.w += kern;
    }
}

//Contains Iterate...Cells methods and ZeroPoint
#include "cl_neighbors.h"

//--------------------------------------------------------------
// compute forces on particles
//#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable
__kernel void colorfield_update(
                        int res,
                       //__global float4* vars_sorted,
                       ARGS,
                       __global int*    cell_indexes_start,
                       __global int*    cell_indexes_end,
                       __constant struct GridParams* gp,
                       __constant struct SPHParams* sphp
                       DEBUG_ARGS
                       )
{
    uint s = get_global_id(0);
    uint t = get_global_id(1);
    uint r = get_global_id(2);
    float4 texPos = (float4)(s/(float)res,t/(float)res,r/(float)res,1.0);
    texPos = (texPos*(gp->grid_max-gp->grid_min)+gp->grid_min)*sphp->simulation_scale;
    texPos.w=1.0f;
    // Do calculations on particles in neighboring cells
    PointData pt;
    zeroPoint(&pt);

    //IterateParticlesInNearbyCells(vars_sorted, &pt, num, index, position_i, cell_indexes_start, cell_indexes_end, gp,/* fp,*/ sphp DEBUG_ARGV);
    IterateParticlesInNearbyCells(ARGV, &pt, 0, 0, texPos, cell_indexes_start, cell_indexes_end, gp,/* fp,*/ sphp DEBUG_ARGV);
    pt.force.x*=sphp->mass;
    img[s+t*res+r*res*res]=pt.force;//write_imagef(posTex,(int4)(s,t,r,0),pt.force);
}

/*-------------------------------------------------------------- */
#endif

