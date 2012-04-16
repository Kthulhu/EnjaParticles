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


#ifndef _CLASSIFYCUBES_CL_
#define _CLASSIFYCUBES_CL_
#include "indexmap.h"
__constant uchar nrOfTriangles[256] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 2, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3, 2, 3, 3, 2, 3, 4, 4, 3, 3, 4, 4, 3, 4, 5, 5, 2, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 4, 2, 3, 3, 4, 3, 4, 2, 3, 3, 4, 4, 5, 4, 5, 3, 2, 3, 4, 4, 3, 4, 5, 3, 2, 4, 5, 5, 4, 5, 2, 4, 1, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3, 2, 3, 3, 4, 3, 4, 4, 5, 3, 2, 4, 3, 4, 3, 5, 2, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 4, 3, 4, 4, 3, 4, 5, 5, 4, 4, 3, 5, 2, 5, 4, 2, 1, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 2, 3, 3, 2, 3, 4, 4, 5, 4, 5, 5, 2, 4, 3, 5, 4, 3, 2, 4, 1, 3, 4, 4, 5, 4, 5, 3, 4, 4, 5, 5, 2, 3, 4, 2, 1, 2, 3, 3, 2, 3, 4, 2, 1, 3, 2, 4, 1, 2, 1, 1, 0};
__kernel void classifyCubes2D(
        __write_only image2d_t histoPyramid, 
        __read_only image2d_t rawData,
        __private unsigned int res,
        __private unsigned int slices,
        __private float isolevel
        ) {
    const int4 p4 = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    const int2 pos = map3Dto2D(p4,res,slices);

    // Find cube class nr
    const float first = read_imagef(rawData, sampler, pos).x;
    const uchar cubeindex = 
    ((first > isolevel)) |
    ((read_imagef(rawData, sampler, map3Dto2D(p4+cubeOffsets[1],res,slices)).x > isolevel) << 1) |
    ((read_imagef(rawData, sampler, map3Dto2D(p4+cubeOffsets[3],res,slices)).x > isolevel) << 2) |
    ((read_imagef(rawData, sampler, map3Dto2D(p4+cubeOffsets[2],res,slices)).x > isolevel) << 3) |
    ((read_imagef(rawData, sampler, map3Dto2D(p4+cubeOffsets[4],res,slices)).x > isolevel) << 4) |
    ((read_imagef(rawData, sampler, map3Dto2D(p4+cubeOffsets[5],res,slices)).x > isolevel) << 5) |
    ((read_imagef(rawData, sampler, map3Dto2D(p4+cubeOffsets[7],res,slices)).x > isolevel) << 6) |
    ((read_imagef(rawData, sampler, map3Dto2D(p4+cubeOffsets[6],res,slices)).x > isolevel) << 7); 

    //int2 poswrite = map3Dto2D(p4,res-1,slices);
    // Store number of triangles
    write_imagef(histoPyramid, pos, (float4)((float)nrOfTriangles[cubeindex], (float)cubeindex, first, 1.0f));
}
#endif
