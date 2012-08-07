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
#define ARGS __global float4* pos, __global float4* vel, __global float4* force, __global float* mass, __global float4* pos_j, __global float4* vel_j, __global float* mass_j, float16 rbParams
#define ARGV pos, vel, force, mass, pos_j, vel_j, mass_j, rbParams

/*----------------------------------------------------------------------*/

#include "cl_sph_macros.h"
#include "cl_sph_structs.h"
//Contains all of the Smoothing Kernels for SPH
#include "cl_sph_kernels.h"

//----------------------------------------------------------------------
/*inline void ForNeighbor(//__global float4*  vars_sorted,
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
    int num = sphp[0].num;

    // get the particle info (in the current grid) to test against
    float4 position_j = pos[index_j] * sphp[0].simulation_scale; 
    float4 r = (position_i - position_j); 
    r.w = 0.f; // I stored density in 4th component
    // |r|
    float rlen = length(r);

    // is this particle within cutoff?
    if (rlen <= sphp[0].smoothing_distance)
    {

        //iej is 0 when we are looking at same particle
        //we allow calculations and just multiply force and xsph
        //by iej to avoid branching
        int iej = index_i != index_j;

        // avoid divide by 0 in Wspiky_dr
        rlen = max(rlen, sphp[0].EPSILON);

        float dWijdr = Wspiky_dr(rlen, sphp[0].smoothing_distance, sphp);

        float di = density[index_i];  // should not repeat di
        float dj = mass[index_j]/(sphp[0].smoothing_distance*sphp[0].smoothing_distance*sphp[0].smoothing_distance);
        float idj = 1.0/dj;

        //form simple SPH in Krog's thesis
        float Pi = sphp[0].K*(di - sphp[0].rest_density);//+sphp[0].rest_density;
        //float Pj = sphp[0].K*(dj - sphp[0].rest_density);//+sphp[0].rest_density;


        float kern = dWijdr * (Pi + Pj) * idj;

        float4 veli = veleval[index_i]; // sorted
        float4 velj = veleval[index_j];

#if 1
        // Add viscous forces
        float dWijlapl = Wvisc_lapl(rlen, sphp[0].smoothing_distance, sphp);
        float4 visc = (velj-veli) * dWijlapl * idj;
        pt[0].viscosity+= visc*(float)iej;

#endif

#if 1
        // Add XSPH stabilization term
        // the poly6 kernel calculation seems to be wrong, using rlen as a vector when it is a float...
        float Wijpol6 = Wpoly6(r, sphp[0].smoothing_distance, sphp);
        //float Wijpol6 = sphp[0].wpoly6_coef * Wpoly6(rlen, sphp[0].smoothing_distance, sphp);
        float4 xsph = (Wijpol6 * (velj-veli)/(di+dj));
        pt[0].xsph += xsph * (float)iej;
        pt[0].xsph.w = 0.f;
#endif

        pt[0].force += r * kern * (float)iej;
        pt[0].force.w = 0.f;

    }
}*/
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
    float4 position_j = pos_j[index_j] * sphp[0].simulation_scale; 
    float4 r = (position_j - position_i); 
    r.w = 0.f; // I stored density in 4th component
    // |r|
    float rlen = length(r);

    // is this particle within cutoff?
    if (rlen <= 2* sphp[0].smoothing_distance)
    {
        // avoid divide by 0 in Wspiky_dr
        rlen = max(rlen, sphp[0].EPSILON);
        float4 norm = r/rlen;
        float massnorm=((mass[index_i]*mass[index_j])/(mass[index_i]+mass[index_j]));
        float stiff=(rbParams.s0*massnorm);
        float4 springForce = -stiff*(2.*sphp[0].smoothing_distance-rlen)*(norm);

        float4 relvel =  vel_j[index_j]-vel[index_i];

        float4 normVel = dot(relvel,norm)*norm;
        float4 dampeningForce = rbParams.s1*sqrt(stiff*massnorm)*(normVel);
        float4 normalForce=(springForce+dampeningForce); 
        //vel[index_i]=dot(vel[index_i],norm)*norm+vel_j[index_j]-dot(vel_j[index_j],norm)*norm;
        
        float4 tanVel = vel[index_i]+normVel;
        //float4 tanForce = -(mass[index_i]*tanVel)/0.003;
        relvel.w=0.0;
        normalForce.w=0.0;
        //Use Gram Schmidt process to find tangential velocity to the particle
        float4 tangVel=relvel-normVel;
        float4 frictionalForce=0.0f;
        //if(length(tangVel)>rbParams.s2)
        //    frictionalForce =-rbParams.s3*length(normalForce)*(normalize(tangVel));
        //    frictionalForce = -rbParams.s3*length(normalForce)*(normalize(tangVel));
        //else
        //    frictionalForce = -rbParams.s4*tangVel;
        pt[0].force += (normalForce+frictionalForce);
        //pt[0].vel
        
        //pt[0].force += normalForce;
    }
}

//Contains Iterate...Cells methods and ZeroPoint
#include "cl_sph_neighbors.h"

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
    int num = sphp[0].num;
    //int numParticles = get_global_size(0);
    //int num = get_global_size(0);

    int index = get_global_id(0);
    if (index >= num) return;

    float4 position_i = pos[index] * sphp[0].simulation_scale;

    //debuging
    clf[index] = (float4)(99,0,0,0);
    //cli[index].w = 0;

    // Do calculations on particles in neighboring cells
    PointData pt;
    zeroPoint(&pt);

    //IterateParticlesInNearbyCells(vars_sorted, &pt, num, index, position_i, cell_indexes_start, cell_indexes_end, gp,/* fp,*/ sphp DEBUG_ARGV);
    IterateParticlesInNearbyCells(ARGV, &pt, num, index, position_i, cell_indexes_start, cell_indexes_end, gp,/* fp,*/ sphp DEBUG_ARGV);
    force[index] += pt.force/sphp[0].mass; 
    clf[index].xyz = pt.force.xyz;
}

/*-------------------------------------------------------------- */
#endif

