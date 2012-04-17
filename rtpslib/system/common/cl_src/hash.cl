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


#ifndef _HASH_CL_H_
#define _HASH_CL_H_



#include "cl_structs.h"
#include "cl_macros.h"
#include "cl_hash.h"

//----------------------------------------------------------------------
// Calculate a grid hash value for each particle


//  Have to make sure that the data associated with the pointers is on the GPU
//struct GridData
//{
//    uint* sort_hashes;          // particle hashes
//    uint* sort_indexes;         // particle indices
//    uint* cell_indexes_start;   // mapping between bucket hash and start index in sorted list
//    uint* cell_indexes_end;     // mapping between bucket hash and end index in sorted list
//};

//----------------------------------------------------------------------
// comes from K_Grid_Hash
// CANNOT USE references to structures/classes as arguments!
__kernel void hash(
                  //__global float4* vars_unsorted,
                  int num,
                  __global float4* pos_u,
                  __global uint* sort_hashes,
                  __global uint* sort_indexes,
                  //__constant struct SPHParams* sphp,
                  __constant struct GridParams* gp
                  DEBUG_ARGS
                  //__global float4* fdebug,
                  //__global int4* idebug
                  )
{
    // particle index
    uint index = get_global_id(0);
    //int num = sphp[0].num;
    //int num = get_global_size(0);
    //comment this out to hash everything if using max_num
    if (index >= num) return;

    // initialize to -1 (used in kernel datastructures in build_datastructures_wrap.cpp
    //int grid_size = (int) (gp[0].grid_res.x*gp[0].grid_res.y*gp[0].grid_res.z);
    //if (index < grid_size) {   // grid_size: 1400
    //cell_indices_start[index] = 0xffffffff; 
    //}

    // particle position
    //float4 p = unsorted_pos(index); // macro
    float4 p = pos_u[index]; // macro

    // get address in grid
    //int4 gridPos = calcGridCell(p, gp[0].grid_min, gp[0].grid_inv_delta);
    int4 gridPos = calcGridCell(p, gp[0].grid_min, gp[0].grid_delta);
    bool wrap_edges = false;
    //uint hash = (uint) calcGridHash(gridPos, gp[0].grid_res, wrap_edges);//, fdebug, idebug);
    int hash = calcGridHash(gridPos, gp[0].grid_res, wrap_edges);//, fdebug, idebug);

    cli[index].xyz = gridPos.xyz;
    cli[index].w = hash;
    //cli[index].w = (gridPos.z*gp[0].grid_res.y + gridPos.y) * gp[0].grid_res.x + gridPos.x; 

    hash = hash > gp[0].nb_cells ? gp[0].nb_cells : hash;
    hash = hash < 0 ? gp[0].nb_cells : hash;
    /*
       //problem is that when we cut num we are hashing the wrong stuff?
    if (index >= num)
    {
        hash = gp[0].nb_cells;
    }
    */
    // store grid hash and particle index
    sort_hashes[index] = (uint)hash;
    //int pp = (int) p.x;

    sort_indexes[index] = index;

    //fdebug[index] = gp[0].grid_inv_delta;
    //fdebug[index] = (float4)((p.x - gp[0].grid_min.x) * gp[0].grid_inv_delta.x, p.x, 0,0);
    //clf[index] = (float4)((p.x - gp[0].grid_min.x) * gp[0].grid_delta.x, p.x, 0,0);
    //clf[index] = p;
    //cli[index].w = sphp[0].max_num;


/*
    clf[0].x = sphp[0].mass;
    clf[0].y = sphp[0].rest_distance;
    clf[0].z = sphp[0].smoothing_distance;
    clf[0].w = sphp[0].simulation_scale;

    clf[1].x = sphp[0].boundary_stiffness;
    clf[1].y = sphp[0].boundary_dampening;
    clf[1].z = sphp[0].boundary_distance;
    clf[1].w = sphp[0].EPSILON;

    clf[2].x = sphp[0].PI;       //delicious
    clf[2].y = sphp[0].K;        //speed of sound
    clf[2].z = sphp[0].viscosity;
    clf[2].w = sphp[0].velocity_limit;

    clf[3].x = sphp[0].xsph_factor;
    clf[3].y = sphp[0].gravity; // -9.8 m/sec^2
    clf[3].z = sphp[0].friction_coef;
    clf[3].w = sphp[0].restitution_coef;

    clf[4].x = sphp[0].shear;
    clf[4].y = sphp[0].attraction;
    clf[4].z = sphp[0].spring;
    //kernel coefficients
    clf[4].w = sphp[0].wpoly6_coef;

    clf[5].x = sphp[0].wpoly6_d_coef;
    clf[5].y = sphp[0].wpoly6_dd_coef; // laplacian
    clf[5].z = sphp[0].wspiky_coef;
    clf[5].w = sphp[0].wspiky_d_coef;

    clf[6].x = sphp[0].wspiky_dd_coef;
    clf[6].y = sphp[0].wvisc_coef;
    clf[6].z = sphp[0].wvisc_d_coef;
    clf[6].w = sphp[0].wvisc_dd_coef;

    clf[7].x = sphp[0].num;
    clf[7].z = sphp[0].choice; // which kind of calculation to invoke
    clf[7].w = sphp[0].max_num;
*/



}
//----------------------------------------------------------------------


#endif
