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
#define ARGS __global float4* pos, __global float* density, __write_only image2d_t img
#define ARGV pos, density, img

/*----------------------------------------------------------------------*/

#include "cl_sph_macros.h"
#include "cl_sph_structs.h"
//Contains all of the Smoothing Kernels for SPH
#include "cl_sph_kernels.h"
inline int2 map3Dto2D(int4 coord, unsigned int res, unsigned int slices)
{
	int yoffset = coord.z/slices;
	int xoffset = coord.z%slices;
	
	int2 pos = {xoffset*res,yoffset*res};
	pos+=(int2)(coord.x,coord.y);
	return pos;
}

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
    float4 position_j = pos[index_j] * sphp[0].simulation_scale;
    float4 r = (position_i - position_j);
    r.w = 0.f; // I stored density in 4th component
    // |r|
    float rlen = length(r);

    // is this particle within cutoff?
    if (rlen <= sphp[0].smoothing_distance)
    {
        // avoid divide by 0 in Wspiky_dr
        rlen = max(rlen, sphp[0].EPSILON);

        float kern = Wspiky(rlen, sphp[0].smoothing_distance, sphp)* (1.0/density[index_j]);

        pt[0].force.x += kern;
        //debugging
        //pt[0].force.w += kern;
    }
}

//Contains Iterate...Cells methods and ZeroPoint
#include "cl_sph_neighbors.h"

//--------------------------------------------------------------
// compute forces on particles
//#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable
__kernel void colorfield_update(
                       unsigned int res,
                       unsigned int slices,
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
    uint index = s+t*res+r*res*res;
    //float4 texPos = (float4)(s/((float)res),t/((float)res),r/((float)res),1.0f);
    float tmp = 1.0f/(res-1);
    //float4 texPos=(float4)(s*tmp,1.0,0.5,1.0f);
    float4 texPos=(float4)(s*tmp,t*tmp,r*tmp,1.0f);
    texPos = (texPos*(gp[0].bnd_max-gp[0].bnd_min)+gp[0].bnd_min);
    //texPos = (texPos*(gp[0].grid_max-gp[0].grid_min)+gp[0].grid_min)*sphp[0].simulation_scale;
    texPos.w=1.0f;
    // Do calculations on particles in neighboring cells
    PointData pt;
    zeroPoint(&pt);

    //IterateParticlesInNearbyCells(vars_sorted, &pt, num, index, position_i, cell_indexes_start, cell_indexes_end, gp,/* fp,*/ sphp DEBUG_ARGV);
    IterateParticlesInNearbyCells(ARGV, &pt, 0, 0, texPos, cell_indexes_start, cell_indexes_end, gp,/* fp,*/ sphp DEBUG_ARGV);
    float tmpiso = pt.force.x*sphp[0].mass;//(pt.force.x>0.0f);//ceil(pt.force.x*sphp[0].mass);
    //write_imagef(img,map3Dto2D((int4)(s,t,r,0),res,slices),(float4)(tmpiso,0.0f,0.0f,1.0f));//,tmpiso,tmpiso,tmpiso));
    write_imagef(img,map3Dto2D((int4)(s,t,r,0),res,slices),tmpiso);//,tmpiso,tmpiso,tmpiso));
    //write_imagef(img,map3Dto2D((int4)(s,t,r,0),res,slices),(float4)(1.0f,1.0f,1.0f,1.0f));
    //int2 mytempdebug=map3Dto2D((int4)(s,t,r,0),res,slices);
    //clf[index]=(float4)(tmpiso,mytempdebug.x,mytempdebug.y,0.0f);//(float4)(s,t,r,index);
    //clf[index]=texPos;
}

/*-------------------------------------------------------------- */
#endif

