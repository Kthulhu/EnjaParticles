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
#define ARGS __global float4* pos, __global float* density, __global float4* veleval, __global float4* force, __global float4* xsph
#define ARGV pos, density, veleval, force, xsph

/*----------------------------------------------------------------------*/

#include "cl_sph_macros.h"
#include "cl_sph_structs.h"
//Contains all of the Smoothing Kernels for SPH
#include "cl_sph_kernels.h"


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
        float dj = density[index_j];
        float idi = 1.0/di;
        float idj = 1.0/dj;

        //form simple SPH in Krog's thesis

        float Pi = sphp[0].K*(di - sphp[0].rest_density);//+sphp[0].rest_density;
        float Pj = sphp[0].K*(dj - sphp[0].rest_density);//+sphp[0].rest_density;

        //need to precalculate B
        //float B=(sphp[0].rest_density*sphp[0].velocity_limit*sphp[0].velocity_limit)/7.0f;
        //float Pi = B*(pow((di/sphp[0].rest_density),7.0f)-1);//+sphp[0].rest_density;
        //float Pj = B*(pow((dj/sphp[0].rest_density),7.0f)-1);//+sphp[0].rest_density;

        //playing with quartic kernel
        //dWijdr = 2.0f/3.0f  - 9.0f * q*q / 8.0f + 19.0f * q*q*q / 24.0f - 5.0f * q*q*q*q / 32.0f; (need derivative of this)
        //quartic_coef = 
        

        float kern = dWijdr * (Pi + Pj) * idj;
        //float kern = dWijdr * (Pj - Pi) * idj * idj;
        //float kern = -.5 * dWijdr * (Pi + Pj) * sphp[0].wspiky_d_coef * idi * idj;
        //float kern = -.5 * dWijdr * (Pi + Pj) * sphp[0].wspiky_d_coef * idj * idj;
        //float kern = -1.0f * dWijdr * (Pi * idi * idi + Pj * idj * idj) * sphp[0].wspiky_d_coef;
        //float4 force = kern*r; 

        float4 veli = veleval[index_i]; // sorted
        float4 velj = veleval[index_j];

#if 1
        // Add viscous forces
        float dWijlapl = Wvisc_lapl(rlen, sphp[0].smoothing_distance, sphp);
        //float4 visc = vvisc * (velj-veli) * dWijlapl * idj * idi;
        float4 visc = (velj-veli) * dWijlapl * idj;
        pt[0].viscosity+= visc*(float)iej;

#endif

        //force *=  sphp[0].mass/(di.x*dj.x);  // original
        //force *= sphp[0].mass;// * idi * idj;
        //force *=  sphp[0].mass;// /(di.x*dj.x); 

#if 1
        // Add XSPH stabilization term
        // the poly6 kernel calculation seems to be wrong, using rlen as a vector when it is a float...
        //float Wijpol6 = Wpoly6(r, sphp[0].smoothing_distance, sphp) * sphp[0].wpoly6_coeff;
        /*
        float h = sphp[0].smoothing_distance;
        float hr2 = (h*h - rlen*rlen);
        float Wijpol6 = hr2*hr2*hr2;// * sphp[0].wpoly6_coeff;
        */
        float Wijpol6 = Wpoly6(r, sphp[0].smoothing_distance, sphp);
        //float Wijpol6 = sphp[0].wpoly6_coef * Wpoly6(rlen, sphp[0].smoothing_distance, sphp);
        float4 xsph = (Wijpol6 * (velj-veli)/(di+dj));
        pt[0].xsph += xsph * (float)iej;
        pt[0].xsph.w = 0.f;
#endif

        pt[0].force += r * kern * (float)iej;
        pt[0].force.w = 0.f;

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
    //force[index] = (sphp[0].mass * (1.0/density[index]))* 
    //               (sphp[0].viscosity * sphp[0].wvisc_dd_coef * pt.viscosity) ; 
    //force[index]-=     (sphp[0].mass * sphp[0].wspiky_d_coef * pt.force); 
    force[index] = (sphp[0].mass * (1.0/density[index]))*
            ((-0.5 * sphp[0].wspiky_d_coef * pt.force) +
                   (sphp[0].viscosity * sphp[0].wvisc_dd_coef * pt.viscosity)) ; 
    clf[index].xyz = pt.force.xyz;
    xsph[index] = 2.0 * sphp[0].mass * sphp[0].wpoly6_coef * pt.xsph;
}

/*-------------------------------------------------------------- */
#endif

