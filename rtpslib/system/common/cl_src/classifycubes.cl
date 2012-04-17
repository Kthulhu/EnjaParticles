/**
Modified original marching cubes opencl implemenation from http://www.thebigblob.com/tag/marching-cubes/
https://github.com/smistad/GPU-Marching-Cubes

Copyright 2011 Erik Smistad. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY Erik Smistad ''AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed
or implied, of Erik Smistad.
*/



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
