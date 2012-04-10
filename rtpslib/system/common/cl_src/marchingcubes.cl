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


//#ifndef _MARCHINGCUBES_CL_
//#define _MARCHINGCUBES_CL_

//#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable

__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;

__constant int4 cubeOffsets[8] = {
        {0, 0, 0, 0},
        {1, 0, 0, 0},
        {0, 0, 1, 0},
        {1, 0, 1, 0},
        {0, 1, 0, 0},
        {1, 1, 0, 0},
        {0, 1, 1, 0},
        {1, 1, 1, 0},
    };
__constant int2 squareOffsets[4] = {
        {0,0},
        {1,0},
        {0,1},
        {1,1}
        };
inline int2 map3Dto2D(int4 coord, unsigned int res, unsigned int slices)
{
    int yoffset = coord.z/slices;
    int xoffset = coord.z%slices;
    
    int2 pos = {xoffset*res,yoffset*res};
    pos+=(int2)(coord.x,coord.y);
    return pos;
}

inline int2 map3Dto2DClamp(int4 coordinate, int4 offset, unsigned int res, unsigned int slices)
{
    int4 coord=coordinate+offset;
    if(coord.z<0||coord.z>res-1)
	coord.z-=offset.z;
    int yoffset = coord.z/slices;
    int xoffset = coord.z%slices;
    
    int2 pos = {xoffset*res,yoffset*res};
    pos+=(int2)(coord.x,coord.y);
    //We need to clamp the address to the edge. Typically this is done
    //for you but because we had to flatten the 3d texture we can't rely
    //hardware clamping.
    int z = (pos.y/res)*slices+(pos.x/res);
    //if the new z value is different then moving in y or x caused us to jump
    //in z due to flatting of the image.
    if(z!=coord.z)
        return map3Dto2D(coordinate,res,slices);
    return pos;
    return map3Dto2D(coord,res,slices);
}

inline int4 map2Dto3D(int2 coord, unsigned int res,unsigned int slices)
{
    int z = (coord.y/res)*slices+(coord.x/res);
    
    int4 pos = {coord.x%res,coord.y%res,z,0};
    return pos;
}

__kernel void constructHPLevel2D(
        __write_only image2d_t writeHistoPyramid,
        __read_only image2d_t readHistoPyramid//,
        //__global float4* clf
    ) {

    int2 coord = {get_global_id(0), get_global_id(1)};
    int2 readPos = coord*2;
    float writeValue = read_imagef(readHistoPyramid, sampler, readPos).x + // 0
        read_imagef(readHistoPyramid, sampler, readPos+squareOffsets[1]).x + // 1
        read_imagef(readHistoPyramid, sampler, readPos+squareOffsets[2]).x + // 2
        read_imagef(readHistoPyramid, sampler, readPos+squareOffsets[3]).x; // 3
    write_imagef(writeHistoPyramid, coord, writeValue);
    //clf[coord.x+coord.y*get_global_size(0)]=(float4)(writeValue,0.0f,0.0f,0.0f);
}
int4 scanHPLevel2D(int target, __read_only image2d_t hp, int4 current) {
    float4 neighbors = {
        read_imagef(hp, sampler, current.xy).x,
        read_imagef(hp, sampler, current.xy + squareOffsets[1]).x,
        read_imagef(hp, sampler, current.xy + squareOffsets[2]).x,
        read_imagef(hp, sampler, current.xy + squareOffsets[3]).x
    };

    float acc = current.w + neighbors.x;
    int4 cmp;
    cmp.x = acc <= target;
    acc += neighbors.y;
    cmp.y = acc <= target;
    acc += neighbors.z;
    cmp.z = acc <= target;
    current.xy += squareOffsets[(cmp.x+cmp.y+cmp.z)];
    current.x = (int)current.x*2;
    current.y = (int)current.y*2;
    current.z = 0;
    current.w = (int)(current.w +
        cmp.x*neighbors.x +
        cmp.y*neighbors.y +
        cmp.z*neighbors.z);
    return current;

}

/*int4 scanHPLevel3D(int target, __read_only image3d_t hp, int4 current) {
    
    int8 neighbors = {
        read_imagei(hp, sampler, current).x,
        read_imagei(hp, sampler, current + cubeOffsets[1]).x,
        read_imagei(hp, sampler, current + cubeOffsets[2]).x,
        read_imagei(hp, sampler, current + cubeOffsets[3]).x,
        read_imagei(hp, sampler, current + cubeOffsets[4]).x,
        read_imagei(hp, sampler, current + cubeOffsets[5]).x,
        read_imagei(hp, sampler, current + cubeOffsets[6]).x,
        read_imagei(hp, sampler, current + cubeOffsets[7]).x
    };

    int acc = current.s3 + neighbors.s0;
    int8 cmp;
    cmp.s0 = acc <= target;
    acc += neighbors.s1;
    cmp.s1 = acc <= target;
    acc += neighbors.s2;
    cmp.s2 = acc <= target;
    acc += neighbors.s3;
    cmp.s3 = acc <= target;
    acc += neighbors.s4;
    cmp.s4 = acc <= target;
    acc += neighbors.s5;
    cmp.s5 = acc <= target;
    acc += neighbors.s6;
    cmp.s6 = acc <= target;
    cmp.s7 = 0;
current.s0 = current.s0*2;
    current.s1 = current.s1*2;
    current.s2 = current.s2*2;
    current.s3 = current.s3 +
        cmp.s0*neighbors.s0 + 
        cmp.s1*neighbors.s1 + 
        cmp.s2*neighbors.s2 + 
        cmp.s3*neighbors.s3 + 
        cmp.s4*neighbors.s4 + 
        cmp.s5*neighbors.s5 + 
        cmp.s6*neighbors.s6 + 
        cmp.s7*neighbors.s7;
    return current;

}*/

__constant char offsets3[72] = {
            // 0
            0,0,0,
            1,0,0,
            // 1
            1,0,0,
            1,0,1,
            // 2
            1,0,1,
            0,0,1,
            // 3
            0,0,1,
            0,0,0,
            // 4
            0,1,0,
            1,1,0,
            // 5
            1,1,0,
            1,1,1,
            // 6
            1,1,1,
            0,1,1,
            // 7
            0,1,1,
            0,1,0,
            // 8
            0,0,0,
            0,1,0,
            // 9
            1,0,0,
            1,1,0,
            // 10
            1,0,1,
            1,1,1,
            // 11
            0,0,1,
            0,1,1
        };

__constant char triTable[4096] =
{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1,
3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1,
3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1,
3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1,
9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1,
1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1,
9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1,
2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1,
8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1,
9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1,
4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1,
3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1,
1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1,
4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1,
4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1,
9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1,
1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1,
5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1,
2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1,
9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1,
0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1,
2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1,
10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1,
4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1,
5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1,
5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1,
9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1,
0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1,
1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1,
10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1,
8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1,
2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1,
7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1,
9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1,
2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1,
11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1,
9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1,
5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1,
11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1,
11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1,
1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1,
9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1,
5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1,
2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1,
0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1,
5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1,
6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1,
0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1,
3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1,
6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1,
5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1,
1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1,
10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1,
6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1,
1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1,
8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1,
7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1,
3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1,
5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1,
0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1,
9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1,
8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1,
5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1,
0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1,
6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1,
10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1,
10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1,
8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1,
1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1,
3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1,
0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1,
10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1,
0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1,
3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1,
6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1,
9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1,
8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1,
3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1,
6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1,
0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1,
10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1,
10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1,
1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1,
2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1,
7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1,
7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1,
2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1,
1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1,
11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1,
8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1,
0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1,
7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1,
10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1,
2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1,
6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1,
7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1,
2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1,
1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1,
10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1,
10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1,
0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1,
7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1,
6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1,
8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1,
9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1,
6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1,
1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1,
4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1,
10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1,
8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1,
0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1,
1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1,
8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1,
10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1,
4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1,
10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1,
5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1,
11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1,
9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1,
6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1,
7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1,
3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1,
7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1,
9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1,
3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1,
6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1,
9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1,
1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1,
4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1,
7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1,
6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1,
3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1,
0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1,
6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1,
1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1,
0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1,
11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1,
6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1,
5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1,
9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1,
1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1,
1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1,
10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1,
0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1,
5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1,
10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1,
11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1,
0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1,
9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1,
7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1,
2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1,
8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1,
9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1,
9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1,
1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1,
9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1,
9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1,
5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1,
0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1,
10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1,
2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1,
0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1,
0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1,
9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1,
5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1,
3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1,
5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1,
8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1,
0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1,
9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1,
0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1,
1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1,
3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1,
4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1,
9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1,
11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1,
11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1,
2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1,
9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1,
3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1,
1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1,
4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1,
4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1,
0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1,
3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1,
3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1,
0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1,
9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1,
1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
void fillVBOs(int4 cubePosition, int2 squarePosition, int target, __private float isolevel,  __read_only image2d_t hp0, __global float* triVBO, __global float* normalVBO, __private unsigned int res, __private unsigned int slices)
{

    char vertexNr = 0;
    const int4 cubeData = convert_int4(read_imagef(hp0, sampler, squarePosition));

    // max 5 triangles
    for(int i = (target-cubePosition.s3)*3; i < (target-cubePosition.s3+1)*3; i++) { // for each vertex in triangle
        const uchar edge = triTable[cubeData.y*16 + i];
        const int4 point0 = (int4)(cubePosition.x + offsets3[edge*6], cubePosition.y + offsets3[edge*6+1], cubePosition.z + offsets3[edge*6+2],0);
        const int4 point1 = (int4)(cubePosition.x + offsets3[edge*6+3], cubePosition.y + offsets3[edge*6+4], cubePosition.z + offsets3[edge*6+5],0);
    int2 p01=map3Dto2DClamp(point0,(int4)(1,0,0,0),res,slices);
    int2 p02=map3Dto2DClamp(point0,(int4)(-1,0,0,0),res,slices);
        // Store vertex in VBO
        
        float4 forwardDifference0;
    forwardDifference0.x=(-read_imagef(hp0, sampler, p01).z+read_imagef(hp0, sampler, p02).z);
    p01=map3Dto2DClamp(point0,(int4)(0,1,0,0),res,slices);
    p02=map3Dto2DClamp(point0,(int4)(0,-1,0,0),res,slices);
    //vstore3((float3)(p01.x,p01.y,0.0f), target*3 + vertexNr, triVBO);
    //vstore3(convert_float3(point0.xyz),target*3 + vertexNr, normalVBO);

    //vstore3((float3)(p02.x,p02.y,0.0f), target*3 + vertexNr, normalVBO);
    forwardDifference0.y=(-read_imagef(hp0, sampler, p01).z+read_imagef(hp0, sampler, p02).z);
    p01=map3Dto2DClamp(point0,(int4)(0,0,1,0),res,slices);
    p02=map3Dto2DClamp(point0,(int4)(0,0,-1,0),res,slices);
    forwardDifference0.z=(-read_imagef(hp0, sampler, p01).z+read_imagef(hp0, sampler, p02).z);
    forwardDifference0.w=0.0f;
        
    int2 p11=map3Dto2DClamp(point1,(int4)(1,0,0,0),res,slices);
    int2 p12=map3Dto2DClamp(point1,(int4)(-1,0,0,0),res,slices);
    float4 forwardDifference1;
    forwardDifference1.x=(-read_imagef(hp0, sampler, p11).z+read_imagef(hp0, sampler, p12).z);
    p11=map3Dto2DClamp(point1,(int4)(0,1,0,0),res,slices);
    p12=map3Dto2DClamp(point1,(int4)(0,-1,0,0),res,slices);
    forwardDifference1.y=(-read_imagef(hp0, sampler, p11).z+read_imagef(hp0, sampler, p12).z);
    p11=map3Dto2DClamp(point1,(int4)(0,0,1,0),res,slices);
    p12=map3Dto2DClamp(point1,(int4)(0,0,-1,0),res,slices);
    forwardDifference1.z=(-read_imagef(hp0, sampler, p11).z+read_imagef(hp0, sampler, p12).z);
    forwardDifference1.w=0.0f;

    int2 p0=map3Dto2D(point0,res,slices);
    int2 p1=map3Dto2D(point1,res,slices);
        const float value0 = read_imagef(hp0, sampler,p0 ).z;
        const float diff = native_divide(
            (isolevel-value0), 
            (read_imagef(hp0, sampler,p1).z - value0));
        //FIXME: 10 should be a parameter which is the scale of the actual render domain.
        const float3 vertex = (mix((float3)(point0.x, point0.y, point0.z), (float3)(point1.x, point1.y, point1.z), diff)/(res-1))*10.0;
        const float3 normal = fast_normalize(mix(forwardDifference0.xyz, forwardDifference1.xyz, diff));
        //const float3 vertex =(((float3)(point0.x, point0.y, point0.z)+ (float3)(point1.x, point1.y, point1.z))/(2.0f*res))*10.0;
        //const float3 normal = fast_normalize((forwardDifference0.xyz+ forwardDifference1.xyz)/2.0f);
        vstore3(vertex, target*3 + vertexNr, triVBO);
        vstore3(normal, target*3 + vertexNr, normalVBO);

        //vstore3(forwardDifference0.xyz, target*3 + vertexNr, triVBO);
        //vstore3(forwardDifference1.xyz, target*3 + vertexNr, normalVBO);
        //vstore3(convert_float3(point0.xyz), target*3 + vertexNr, triVBO);
        //vstore3(convert_float3(cubePosition.xyz), target*3 + vertexNr, normalVBO);

        ++vertexNr;
    }

}
__kernel void traverseHP2D16(
        __read_only image2d_t hp0, // Largest HP
        __read_only image2d_t hp1,
        __read_only image2d_t hp2,
        __read_only image2d_t hp3,
        __read_only image2d_t hp4,
        __read_only image2d_t hp5,
        __read_only image2d_t hp6,
        __read_only image2d_t hp7,
        __read_only image2d_t hp8, 
        __read_only image2d_t hp9, 
        __read_only image2d_t hp10, 
        __read_only image2d_t hp11, 
        __read_only image2d_t hp12, 
        __read_only image2d_t hp13, 
        __read_only image2d_t hp14, 
        __read_only image2d_t hp15, 
        __global float* triVBO,
        __global float* normalVBO,
        __private unsigned int res,
        __private unsigned int slices,
        __private float isolevel,
        __private int sum
        ) {
    
    int target = get_global_id(0);
    if(target >= sum)
    target = 0;

    int4 cubePosition = {0,0,0,0}; // x,y,z,sum
    cubePosition = scanHPLevel2D(target, hp15, cubePosition);
    cubePosition = scanHPLevel2D(target, hp14, cubePosition);
    cubePosition = scanHPLevel2D(target, hp13, cubePosition);
    cubePosition = scanHPLevel2D(target, hp12, cubePosition);
    cubePosition = scanHPLevel2D(target, hp11, cubePosition);
    cubePosition = scanHPLevel2D(target, hp10, cubePosition);
    cubePosition = scanHPLevel2D(target, hp9, cubePosition);
    cubePosition = scanHPLevel2D(target, hp8, cubePosition);
    cubePosition = scanHPLevel2D(target, hp7, cubePosition);
    cubePosition = scanHPLevel2D(target, hp6, cubePosition);
    cubePosition = scanHPLevel2D(target, hp5, cubePosition);
    cubePosition = scanHPLevel2D(target, hp4, cubePosition);
    cubePosition = scanHPLevel2D(target, hp3, cubePosition);
    cubePosition = scanHPLevel2D(target, hp2, cubePosition);
    cubePosition = scanHPLevel2D(target, hp1, cubePosition);
    cubePosition = scanHPLevel2D(target, hp0, cubePosition);
    cubePosition.x=cubePosition.x/2;
    cubePosition.y=cubePosition.y/2;
    int2 squarePos = cubePosition.xy;
    cubePosition.xyz=map2Dto3D(squarePos,res,slices).xyz;

    fillVBOs(cubePosition,squarePos,target,isolevel,hp0, triVBO, normalVBO, res, slices);
}

__kernel void traverseHP2D15(
        __read_only image2d_t hp0, // Largest HP
        __read_only image2d_t hp1,
        __read_only image2d_t hp2,
        __read_only image2d_t hp3,
        __read_only image2d_t hp4,
        __read_only image2d_t hp5,
        __read_only image2d_t hp6,
        __read_only image2d_t hp7,
        __read_only image2d_t hp8, 
        __read_only image2d_t hp9, 
        __read_only image2d_t hp10, 
        __read_only image2d_t hp11, 
        __read_only image2d_t hp12, 
        __read_only image2d_t hp13, 
        __read_only image2d_t hp14, 
        __global float* triVBO,
        __global float* normalVBO,
        __private unsigned int res,
        __private unsigned int slices,
        __private float isolevel,
        __private int sum
        ) {
    
    int target = get_global_id(0);
    if(target >= sum)
    target = 0;

    int4 cubePosition = {0,0,0,0}; // x,y,z,sum
    cubePosition = scanHPLevel2D(target, hp14, cubePosition);
    cubePosition = scanHPLevel2D(target, hp13, cubePosition);
    cubePosition = scanHPLevel2D(target, hp12, cubePosition);
    cubePosition = scanHPLevel2D(target, hp11, cubePosition);
    cubePosition = scanHPLevel2D(target, hp10, cubePosition);
    cubePosition = scanHPLevel2D(target, hp9, cubePosition);
    cubePosition = scanHPLevel2D(target, hp8, cubePosition);
    cubePosition = scanHPLevel2D(target, hp7, cubePosition);
    cubePosition = scanHPLevel2D(target, hp6, cubePosition);
    cubePosition = scanHPLevel2D(target, hp5, cubePosition);
    cubePosition = scanHPLevel2D(target, hp4, cubePosition);
    cubePosition = scanHPLevel2D(target, hp3, cubePosition);
    cubePosition = scanHPLevel2D(target, hp2, cubePosition);
    cubePosition = scanHPLevel2D(target, hp1, cubePosition);
    cubePosition = scanHPLevel2D(target, hp0, cubePosition);
    cubePosition.x=cubePosition.x/2;
    cubePosition.y=cubePosition.y/2;
    int2 squarePos = cubePosition.xy;
    cubePosition.xyz=map2Dto3D(squarePos,res,slices).xyz;

    fillVBOs(cubePosition,squarePos,target,isolevel,hp0, triVBO, normalVBO, res, slices);
}
__kernel void traverseHP2D14(
        __read_only image2d_t hp0, // Largest HP
        __read_only image2d_t hp1,
        __read_only image2d_t hp2,
        __read_only image2d_t hp3,
        __read_only image2d_t hp4,
        __read_only image2d_t hp5,
        __read_only image2d_t hp6,
        __read_only image2d_t hp7,
        __read_only image2d_t hp8, 
        __read_only image2d_t hp9, 
        __read_only image2d_t hp10, 
        __read_only image2d_t hp11, 
        __read_only image2d_t hp12, 
        __read_only image2d_t hp13, 
        __global float* triVBO,
        __global float* normalVBO,
        __private unsigned int res,
        __private unsigned int slices,
        __private float isolevel,
        __private int sum
        ) {
    
    int target = get_global_id(0);
    if(target >= sum)
    target = 0;

    int4 cubePosition = {0,0,0,0}; // x,y,z,sum
    cubePosition = scanHPLevel2D(target, hp13, cubePosition);
    cubePosition = scanHPLevel2D(target, hp12, cubePosition);
    cubePosition = scanHPLevel2D(target, hp11, cubePosition);
    cubePosition = scanHPLevel2D(target, hp10, cubePosition);
    cubePosition = scanHPLevel2D(target, hp9, cubePosition);
    cubePosition = scanHPLevel2D(target, hp8, cubePosition);
    cubePosition = scanHPLevel2D(target, hp7, cubePosition);
    cubePosition = scanHPLevel2D(target, hp6, cubePosition);
    cubePosition = scanHPLevel2D(target, hp5, cubePosition);
    cubePosition = scanHPLevel2D(target, hp4, cubePosition);
    cubePosition = scanHPLevel2D(target, hp3, cubePosition);
    cubePosition = scanHPLevel2D(target, hp2, cubePosition);
    cubePosition = scanHPLevel2D(target, hp1, cubePosition);
    cubePosition = scanHPLevel2D(target, hp0, cubePosition);
    cubePosition.x=cubePosition.x/2;
    cubePosition.y=cubePosition.y/2;
    int2 squarePos = cubePosition.xy;
    cubePosition.xyz=map2Dto3D(squarePos,res,slices).xyz;

    fillVBOs(cubePosition,squarePos,target,isolevel,hp0, triVBO, normalVBO, res, slices);

}
__kernel void traverseHP2D13(
        __read_only image2d_t hp0, // Largest HP
        __read_only image2d_t hp1,
        __read_only image2d_t hp2,
        __read_only image2d_t hp3,
        __read_only image2d_t hp4,
        __read_only image2d_t hp5,
        __read_only image2d_t hp6,
        __read_only image2d_t hp7,
        __read_only image2d_t hp8, 
        __read_only image2d_t hp9, 
        __read_only image2d_t hp10, 
        __read_only image2d_t hp11, 
        __read_only image2d_t hp12, 
        __global float* triVBO,
        __global float* normalVBO,
        __private unsigned int res,
        __private unsigned int slices,
        __private float isolevel,
        __private int sum
        ) {
    
    int target = get_global_id(0);
    if(target >= sum)
    target = 0;

    int4 cubePosition = {0,0,0,0}; // x,y,z,sum
    cubePosition = scanHPLevel2D(target, hp12, cubePosition);
    cubePosition = scanHPLevel2D(target, hp11, cubePosition);
    cubePosition = scanHPLevel2D(target, hp10, cubePosition);
    cubePosition = scanHPLevel2D(target, hp9, cubePosition);
    cubePosition = scanHPLevel2D(target, hp8, cubePosition);
    cubePosition = scanHPLevel2D(target, hp7, cubePosition);
    cubePosition = scanHPLevel2D(target, hp6, cubePosition);
    cubePosition = scanHPLevel2D(target, hp5, cubePosition);
    cubePosition = scanHPLevel2D(target, hp4, cubePosition);
    cubePosition = scanHPLevel2D(target, hp3, cubePosition);
    cubePosition = scanHPLevel2D(target, hp2, cubePosition);
    cubePosition = scanHPLevel2D(target, hp1, cubePosition);
    cubePosition = scanHPLevel2D(target, hp0, cubePosition);
    cubePosition.x=cubePosition.x/2;
    cubePosition.y=cubePosition.y/2;
    int2 squarePos = cubePosition.xy;
    cubePosition.xyz=map2Dto3D(squarePos,res,slices).xyz;

    fillVBOs(cubePosition,squarePos,target,isolevel,hp0, triVBO, normalVBO, res, slices);

}
__kernel void traverseHP2D12(
        __read_only image2d_t hp0, // Largest HP
        __read_only image2d_t hp1,
        __read_only image2d_t hp2,
        __read_only image2d_t hp3,
        __read_only image2d_t hp4,
        __read_only image2d_t hp5,
        __read_only image2d_t hp6,
        __read_only image2d_t hp7,
        __read_only image2d_t hp8, 
        __read_only image2d_t hp9, 
        __read_only image2d_t hp10, 
        __read_only image2d_t hp11, 
        __global float* triVBO,
        __global float* normalVBO,
        __private unsigned int res,
        __private unsigned int slices,
        __private float isolevel,
        __private int sum
        ) {
    
    int target = get_global_id(0);
    if(target >= sum)
    target = 0;

    int4 cubePosition = {0,0,0,0}; // x,y,z,sum
    cubePosition = scanHPLevel2D(target, hp11, cubePosition);
    cubePosition = scanHPLevel2D(target, hp10, cubePosition);
    cubePosition = scanHPLevel2D(target, hp9, cubePosition);
    cubePosition = scanHPLevel2D(target, hp8, cubePosition);
    cubePosition = scanHPLevel2D(target, hp7, cubePosition);
    cubePosition = scanHPLevel2D(target, hp6, cubePosition);
    cubePosition = scanHPLevel2D(target, hp5, cubePosition);
    cubePosition = scanHPLevel2D(target, hp4, cubePosition);
    cubePosition = scanHPLevel2D(target, hp3, cubePosition);
    cubePosition = scanHPLevel2D(target, hp2, cubePosition);
    cubePosition = scanHPLevel2D(target, hp1, cubePosition);
    cubePosition = scanHPLevel2D(target, hp0, cubePosition);
    cubePosition.x=cubePosition.x/2;
    cubePosition.y=cubePosition.y/2;
    int2 squarePos = cubePosition.xy;
    cubePosition.xyz=map2Dto3D(squarePos,res,slices).xyz;

    fillVBOs(cubePosition,squarePos,target,isolevel,hp0, triVBO, normalVBO, res, slices);

}
__kernel void traverseHP2D11(
        __read_only image2d_t hp0, // Largest HP
        __read_only image2d_t hp1,
        __read_only image2d_t hp2,
        __read_only image2d_t hp3,
        __read_only image2d_t hp4,
        __read_only image2d_t hp5,
        __read_only image2d_t hp6,
        __read_only image2d_t hp7,
        __read_only image2d_t hp8, 
        __read_only image2d_t hp9, 
        __read_only image2d_t hp10, 
        __global float* triVBO,
        __global float* normalVBO,
        __private unsigned int res,
        __private unsigned int slices,
        __private float isolevel,
        __private int sum
        ) {
    
    int target = get_global_id(0);
    if(target >= sum)
    target = 0;

    int4 cubePosition = {0,0,0,0}; // x,y,z,sum
    cubePosition = scanHPLevel2D(target, hp10, cubePosition);
    cubePosition = scanHPLevel2D(target, hp9, cubePosition);
    cubePosition = scanHPLevel2D(target, hp8, cubePosition);
    cubePosition = scanHPLevel2D(target, hp7, cubePosition);
    cubePosition = scanHPLevel2D(target, hp6, cubePosition);
    cubePosition = scanHPLevel2D(target, hp5, cubePosition);
    cubePosition = scanHPLevel2D(target, hp4, cubePosition);
    cubePosition = scanHPLevel2D(target, hp3, cubePosition);
    cubePosition = scanHPLevel2D(target, hp2, cubePosition);
    cubePosition = scanHPLevel2D(target, hp1, cubePosition);
    cubePosition = scanHPLevel2D(target, hp0, cubePosition);
    cubePosition.x=cubePosition.x/2;
    cubePosition.y=cubePosition.y/2;
    int2 squarePos = cubePosition.xy;
    cubePosition.xyz=map2Dto3D(squarePos,res,slices).xyz;

    fillVBOs(cubePosition,squarePos,target,isolevel,hp0, triVBO, normalVBO, res, slices);

}
__kernel void traverseHP2D10(
        __read_only image2d_t hp0, // Largest HP
        __read_only image2d_t hp1,
        __read_only image2d_t hp2,
        __read_only image2d_t hp3,
        __read_only image2d_t hp4,
        __read_only image2d_t hp5,
        __read_only image2d_t hp6,
        __read_only image2d_t hp7,
        __read_only image2d_t hp8, 
        __read_only image2d_t hp9, 
        __global float* triVBO,
        __global float* normalVBO,
        __private unsigned int res,
        __private unsigned int slices,
        __private float isolevel,
        __private int sum
        ) {
    
    int target = get_global_id(0);
    if(target >= sum)
    target = 0;

    int4 cubePosition = {0,0,0,0}; // x,y,z,sum
    cubePosition = scanHPLevel2D(target, hp9, cubePosition);
    cubePosition = scanHPLevel2D(target, hp8, cubePosition);
    cubePosition = scanHPLevel2D(target, hp7, cubePosition);
    cubePosition = scanHPLevel2D(target, hp6, cubePosition);
    cubePosition = scanHPLevel2D(target, hp5, cubePosition);
    cubePosition = scanHPLevel2D(target, hp4, cubePosition);
    cubePosition = scanHPLevel2D(target, hp3, cubePosition);
    cubePosition = scanHPLevel2D(target, hp2, cubePosition);
    cubePosition = scanHPLevel2D(target, hp1, cubePosition);
    cubePosition = scanHPLevel2D(target, hp0, cubePosition);
    cubePosition.x=cubePosition.x/2;
    cubePosition.y=cubePosition.y/2;
    int2 squarePos = cubePosition.xy;
    cubePosition.xyz=map2Dto3D(squarePos,res,slices).xyz;

    fillVBOs(cubePosition,squarePos,target,isolevel,hp0, triVBO, normalVBO, res, slices);

}
__kernel void traverseHP2D9(
        __read_only image2d_t hp0, // Largest HP
        __read_only image2d_t hp1,
        __read_only image2d_t hp2,
        __read_only image2d_t hp3,
        __read_only image2d_t hp4,
        __read_only image2d_t hp5,
        __read_only image2d_t hp6,
        __read_only image2d_t hp7,
        __read_only image2d_t hp8, 
        __global float* triVBO,
        __global float* normalVBO,
        __private unsigned int res,
        __private unsigned int slices,
        __private float isolevel,
        __private int sum
        ) {
    
    int target = get_global_id(0);
    if(target >= sum)
    target = 0;

    int4 cubePosition = {0,0,0,0}; // x,y,z,sum
    cubePosition = scanHPLevel2D(target, hp8, cubePosition);
    cubePosition = scanHPLevel2D(target, hp7, cubePosition);
    cubePosition = scanHPLevel2D(target, hp6, cubePosition);
    cubePosition = scanHPLevel2D(target, hp5, cubePosition);
    cubePosition = scanHPLevel2D(target, hp4, cubePosition);
    cubePosition = scanHPLevel2D(target, hp3, cubePosition);
    cubePosition = scanHPLevel2D(target, hp2, cubePosition);
    cubePosition = scanHPLevel2D(target, hp1, cubePosition);
    cubePosition = scanHPLevel2D(target, hp0, cubePosition);
    cubePosition.x=cubePosition.x/2;
    cubePosition.y=cubePosition.y/2;
    int2 squarePos = cubePosition.xy;
    cubePosition.xyz=map2Dto3D(squarePos,res,slices).xyz;

    fillVBOs(cubePosition,squarePos,target,isolevel,hp0, triVBO, normalVBO, res, slices);

}
__kernel void traverseHP2D8(
        __read_only image2d_t hp0, // Largest HP
        __read_only image2d_t hp1,
        __read_only image2d_t hp2,
        __read_only image2d_t hp3,
        __read_only image2d_t hp4,
        __read_only image2d_t hp5,
        __read_only image2d_t hp6,
        __read_only image2d_t hp7,
        __global float* triVBO,
        __global float* normalVBO,
        __private unsigned int res,
        __private unsigned int slices,
        __private float isolevel,
        __private unsigned int sum
        ) {
    
    int target = get_global_id(0);
    if(target >= sum)
    target = 0;

    int4 cubePosition = {0,0,0,0}; // x,y,z,sum
    cubePosition = scanHPLevel2D(target, hp7, cubePosition);
    cubePosition = scanHPLevel2D(target, hp6, cubePosition);
    cubePosition = scanHPLevel2D(target, hp5, cubePosition);
    cubePosition = scanHPLevel2D(target, hp4, cubePosition);
    cubePosition = scanHPLevel2D(target, hp3, cubePosition);
    cubePosition = scanHPLevel2D(target, hp2, cubePosition);
    cubePosition = scanHPLevel2D(target, hp1, cubePosition);
    cubePosition = scanHPLevel2D(target, hp0, cubePosition);
    cubePosition.x=cubePosition.x/2;
    cubePosition.y=cubePosition.y/2;
    int2 squarePos = cubePosition.xy;
    cubePosition.xyz=map2Dto3D(squarePos,res,slices).xyz;
    //DEBUGGING
    //vstore3((float3)(squarePosition.x, squarePosition.y,0),target*3, triVBO);
    //vstore3(convert_float3(cubePosition.xyz), target*3, normalVBO);


    fillVBOs(cubePosition,squarePos,target,isolevel,hp0, triVBO, normalVBO, res, slices);
}

__kernel void traverseHP2D7(
        __read_only image2d_t hp0, // Largest HP
        __read_only image2d_t hp1,
        __read_only image2d_t hp2,
        __read_only image2d_t hp3,
        __read_only image2d_t hp4,
        __read_only image2d_t hp5,
        __read_only image2d_t hp6,
        __global float* triVBO,
        __global float* normalVBO,
        __private unsigned int res,
        __private unsigned int slices,
        __private float isolevel,
        __private int sum
        ) {
    
    int target = get_global_id(0);
    if(target >= sum)
    target = 0;

    int4 cubePosition = {0,0,0,0}; // x,y,z,sum
    cubePosition = scanHPLevel2D(target, hp6, cubePosition);
    cubePosition = scanHPLevel2D(target, hp5, cubePosition);
    cubePosition = scanHPLevel2D(target, hp4, cubePosition);
    cubePosition = scanHPLevel2D(target, hp3, cubePosition);
    cubePosition = scanHPLevel2D(target, hp2, cubePosition);
    cubePosition = scanHPLevel2D(target, hp1, cubePosition);
    cubePosition = scanHPLevel2D(target, hp0, cubePosition);
    cubePosition.x=cubePosition.x/2;
    cubePosition.y=cubePosition.y/2;
    int2 squarePos = cubePosition.xy;
    cubePosition.xyz=map2Dto3D(squarePos,res,slices).xyz;

    fillVBOs(cubePosition,squarePos,target,isolevel,hp0, triVBO, normalVBO, res, slices);

}

__kernel void traverseHP2D6(
        __read_only image2d_t hp0, // Largest HP
        __read_only image2d_t hp1,
        __read_only image2d_t hp2,
        __read_only image2d_t hp3,
        __read_only image2d_t hp4,
        __read_only image2d_t hp5,
        __global float* triVBO,
        __global float* normalVBO,
        __private unsigned int res,
        __private unsigned int slices,
        __private float isolevel,
        __private int sum
        ) {
    
    int target = get_global_id(0);
    if(target >= sum)
    target = 0;

    int4 cubePosition = {0,0,0,0}; // x,y,z,sum
    cubePosition = scanHPLevel2D(target, hp5, cubePosition);
    cubePosition = scanHPLevel2D(target, hp4, cubePosition);
    cubePosition = scanHPLevel2D(target, hp3, cubePosition);
    cubePosition = scanHPLevel2D(target, hp2, cubePosition);
    cubePosition = scanHPLevel2D(target, hp1, cubePosition);
    cubePosition = scanHPLevel2D(target, hp0, cubePosition);
    cubePosition.x=cubePosition.x/2;
    cubePosition.y=cubePosition.y/2;
    int2 squarePos = cubePosition.xy;
    cubePosition.xyz=map2Dto3D(squarePos,res,slices).xyz;

    fillVBOs(cubePosition,squarePos,target,isolevel,hp0, triVBO, normalVBO, res, slices);

}

__kernel void traverseHP2D5(
        __read_only image2d_t hp0, // Largest HP
        __read_only image2d_t hp1,
        __read_only image2d_t hp2,
        __read_only image2d_t hp3,
        __read_only image2d_t hp4,
        __global float* triVBO,
        __global float* normalVBO,
        __private unsigned int res,
        __private unsigned int slices,
        __private float isolevel,
        __private int sum
        ) {
    
    int target = get_global_id(0);
    if(target >= sum)
    target = 0;

    int4 cubePosition = {0,0,0,0}; // x,y,z,sum
    cubePosition = scanHPLevel2D(target, hp4, cubePosition);
    cubePosition = scanHPLevel2D(target, hp3, cubePosition);
    cubePosition = scanHPLevel2D(target, hp2, cubePosition);
    cubePosition = scanHPLevel2D(target, hp1, cubePosition);
    cubePosition = scanHPLevel2D(target, hp0, cubePosition);
    cubePosition.x=cubePosition.x/2;
    cubePosition.y=cubePosition.y/2;
    int2 squarePos = cubePosition.xy;
    cubePosition.xyz=map2Dto3D(squarePos,res,slices).xyz;

    fillVBOs(cubePosition,squarePos,target,isolevel,hp0, triVBO, normalVBO, res, slices);

}
__kernel void traverseHP2D4(
        __read_only image2d_t hp0, // Largest HP
        __read_only image2d_t hp1,
        __read_only image2d_t hp2,
        __read_only image2d_t hp3,
        __global float* triVBO,
        __global float* normalVBO,
        __private unsigned int res,
        __private unsigned int slices,
        __private float isolevel,
        __private int sum
        ) {
    
    int target = get_global_id(0);
    if(target >= sum)
    target = 0;

    int4 cubePosition = {0,0,0,0}; // x,y,z,sum
    cubePosition = scanHPLevel2D(target, hp3, cubePosition);
    cubePosition = scanHPLevel2D(target, hp2, cubePosition);
    cubePosition = scanHPLevel2D(target, hp1, cubePosition);
    cubePosition = scanHPLevel2D(target, hp0, cubePosition);
    cubePosition.x=cubePosition.x/2;
    cubePosition.y=cubePosition.y/2;
    int2 squarePos = cubePosition.xy;
    cubePosition.xyz=map2Dto3D(squarePos,res,slices).xyz;

    fillVBOs(cubePosition,squarePos,target,isolevel,hp0, triVBO, normalVBO, res, slices);

}

__kernel void traverseHP2D3(
        __read_only image2d_t hp0, // Largest HP
        __read_only image2d_t hp1,
        __read_only image2d_t hp2,
        __global float* triVBO,
        __global float* normalVBO,
        __private unsigned int res,
        __private unsigned int slices,
        __private float isolevel,
        __private int sum
        ) {
    
    int target = get_global_id(0);
    if(target >= sum)
    target = 0;

    int4 cubePosition = {0,0,0,0}; // x,y,z,sum
    cubePosition = scanHPLevel2D(target, hp2, cubePosition);
    cubePosition = scanHPLevel2D(target, hp1, cubePosition);
    cubePosition = scanHPLevel2D(target, hp0, cubePosition);
    cubePosition.x=cubePosition.x/2;
    cubePosition.y=cubePosition.y/2;
    int2 squarePos = cubePosition.xy;
    cubePosition.xyz=map2Dto3D(squarePos,res,slices).xyz;

    fillVBOs(cubePosition,squarePos,target,isolevel,hp0, triVBO, normalVBO, res, slices);

}

__kernel void traverseHP2D2(
        __read_only image2d_t hp0, // Largest HP
        __read_only image2d_t hp1,
        __global float* triVBO,
        __global float* normalVBO,
        __private unsigned int res,
        __private unsigned int slices,
        __private float isolevel,
        __private int sum
        ) {
    
    int target = get_global_id(0);
    if(target >= sum)
    target = 0;

    int4 cubePosition = {0,0,0,0}; // x,y,z,sum
    cubePosition = scanHPLevel2D(target, hp1, cubePosition);
    cubePosition = scanHPLevel2D(target, hp0, cubePosition);
    cubePosition.x=cubePosition.x/2;
    cubePosition.y=cubePosition.y/2;
    int2 squarePos = cubePosition.xy;
    cubePosition.xyz=map2Dto3D(squarePos,res,slices).xyz;

    fillVBOs(cubePosition,squarePos,target,isolevel,hp0, triVBO, normalVBO, res, slices);

}

__kernel void traverseHP2D1(
        __read_only image2d_t hp0, // Largest HP
        __global float* triVBO,
        __global float* normalVBO,
        __private unsigned int res,
        __private unsigned int slices,
        __private float isolevel,
        __private int sum
        ) {
    
    int target = get_global_id(0);
    if(target >= sum)
        target = 0;

    int4 cubePosition = {0,0,0,0}; // x,y,z,sum
    cubePosition = scanHPLevel2D(target, hp0, cubePosition);
    cubePosition.x=cubePosition.x/2;
    cubePosition.y=cubePosition.y/2;
    int2 squarePos = cubePosition.xy;
    cubePosition.xyz=map2Dto3D(squarePos,res,slices).xyz;

    fillVBOs(cubePosition,squarePos,target,isolevel,hp0, triVBO, normalVBO, res, slices);

}


/*__kernel void traverseHP3D(
        __read_only image3d_t hp0, // Largest HP
        __read_only image3d_t hp1,
        __read_only image3d_t hp2,
        __read_only image3d_t hp3,
        __read_only image3d_t hp4,
        __read_only image3d_t hp5,
        #if SIZE > 64
        __read_only image3d_t hp6,
        #endif
        #if SIZE > 128
        __read_only image3d_t hp7,
        #endif
        #if SIZE > 256
        __read_only image3d_t hp8, 
        #endif
        #if SIZE > 512
        __read_only image3d_t hp9, 
        #endif
        __global float * VBOBuffer,
        __private float isolevel,
        __private int sum
        ) {
    
    int target = get_global_id(0);
    if(target >= sum)
        target = 0;

    int4 cubePosition = {0,0,0,0}; // x,y,z,sum
    #if SIZE > 512
    cubePosition = scanHPLevel(target, hp9, cubePosition);
    #endif
    #if SIZE > 256
    cubePosition = scanHPLevel(target, hp8, cubePosition);
    #endif
    #if SIZE > 128
    cubePosition = scanHPLevel(target, hp7, cubePosition);
    #endif
    #if SIZE > 64
    cubePosition = scanHPLevel(target, hp6, cubePosition);
    #endif
    cubePosition = scanHPLevel(target, hp5, cubePosition);
    cubePosition = scanHPLevel(target, hp4, cubePosition);
    cubePosition = scanHPLevel(target, hp3, cubePosition);
    cubePosition = scanHPLevel(target, hp2, cubePosition);
    cubePosition = scanHPLevel(target, hp1, cubePosition);
    cubePosition = scanHPLevel(target, hp0, cubePosition);
    cubePosition.x = cubePosition.x / 2;
    cubePosition.y = cubePosition.y / 2;
    cubePosition.z = cubePosition.z / 2;

    char vertexNr = 0;
    const int4 cubeData = read_imagei(hp0, sampler, cubePosition);

    // max 5 triangles
    for(int i = (target-cubePosition.s3)*3; i < (target-cubePosition.s3+1)*3; i++) { // for each vertex in triangle
        const uchar edge = triTable[cubeData.y*16 + i];
        const int3 point0 = (int3)(cubePosition.x + offsets3[edge*6], cubePosition.y + offsets3[edge*6+1], cubePosition.z + offsets3[edge*6+2]);
        const int3 point1 = (int3)(cubePosition.x + offsets3[edge*6+3], cubePosition.y + offsets3[edge*6+4], cubePosition.z + offsets3[edge*6+5]);

        // Store vertex in VBO
        
        const float3 forwardDifference0 = (float3)(
                (float)(-read_imagei(hp0, sampler, (int4)(point0.x+1, point0.y, point0.z, 0)).z+read_imagei(hp0, sampler, (int4)(point0.x-1, point0.y, point0.z, 0)).z),
                (float)(-read_imagei(hp0, sampler, (int4)(point0.x, point0.y+1, point0.z, 0)).z+read_imagei(hp0, sampler, (int4)(point0.x, point0.y-1, point0.z, 0)).z),
                (float)(-read_imagei(hp0, sampler, (int4)(point0.x, point0.y, point0.z+1, 0)).z+read_imagei(hp0, sampler, (int4)(point0.x, point0.y, point0.z-1, 0)).z)
            );
        const float3 forwardDifference1 = (float3)(
                (float)(-read_imagei(hp0, sampler, (int4)(point1.x+1, point1.y, point1.z, 0)).z+read_imagei(hp0, sampler, (int4)(point1.x-1, point1.y, point1.z, 0)).z),
                (float)(-read_imagei(hp0, sampler, (int4)(point1.x, point1.y+1, point1.z, 0)).z+read_imagei(hp0, sampler, (int4)(point1.x, point1.y-1, point1.z, 0)).z),
                (float)(-read_imagei(hp0, sampler, (int4)(point1.x, point1.y, point1.z+1, 0)).z+read_imagei(hp0, sampler, (int4)(point1.x, point1.y, point1.z-1, 0)).z)
            );

        const int value0 = read_imagei(hp0, sampler, (int4)(point0.x, point0.y, point0.z, 0)).z;
        const float diff = native_divide(
            (float)(isolevel-value0), 
            (float)(read_imagei(hp0, sampler, (int4)(point1.x, point1.y, point1.z, 0)).z - value0));
        
        const float3 vertex = mix((float3)(point0.x, point0.y, point0.z), (float3)(point1.x, point1.y, point1.z), diff);

        const float3 normal = fast_normalize(mix(forwardDifference0, forwardDifference1, diff));


        vstore3(vertex, target*6 + vertexNr*2, VBOBuffer);
        vstore3(normal, target*6 + vertexNr*2 + 1, VBOBuffer);


        ++vertexNr;
    }
}*/

__constant uchar nrOfTriangles[256] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 2, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3, 2, 3, 3, 2, 3, 4, 4, 3, 3, 4, 4, 3, 4, 5, 5, 2, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 4, 2, 3, 3, 4, 3, 4, 2, 3, 3, 4, 4, 5, 4, 5, 3, 2, 3, 4, 4, 3, 4, 5, 3, 2, 4, 5, 5, 4, 5, 2, 4, 1, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 3, 2, 3, 3, 4, 3, 4, 4, 5, 3, 2, 4, 3, 4, 3, 5, 2, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 4, 3, 4, 4, 3, 4, 5, 5, 4, 4, 3, 5, 2, 5, 4, 2, 1, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 2, 3, 3, 2, 3, 4, 4, 5, 4, 5, 5, 2, 4, 3, 5, 4, 3, 2, 4, 1, 3, 4, 4, 5, 4, 5, 3, 4, 4, 5, 5, 2, 3, 4, 2, 1, 2, 3, 3, 2, 3, 4, 2, 1, 3, 2, 4, 1, 2, 1, 1, 0};

/*__kernel void classifyCubes3D(
        __write_only image3d_t histoPyramid, 
        __read_only image3d_t rawData,
        __private int isolevel
        ) {
    int4 pos = {get_global_id(0), get_global_id(1), get_global_id(2), 0};

    // Find cube class nr
    const uchar first = read_imagei(rawData, sampler, pos).x;
    const uchar cubeindex = 
    ((first > isolevel)) |
    ((read_imagei(rawData, sampler, pos + cubeOffsets[1]).x > isolevel) << 1) |
    ((read_imagei(rawData, sampler, pos + cubeOffsets[3]).x > isolevel) << 2) |
    ((read_imagei(rawData, sampler, pos + cubeOffsets[2]).x > isolevel) << 3) |
    ((read_imagei(rawData, sampler, pos + cubeOffsets[4]).x > isolevel) << 4) |
    ((read_imagei(rawData, sampler, pos + cubeOffsets[5]).x > isolevel) << 5) |
    ((read_imagei(rawData, sampler, pos + cubeOffsets[7]).x > isolevel) << 6) |
    ((read_imagei(rawData, sampler, pos + cubeOffsets[6]).x > isolevel) << 7);

    // Store number of triangles
    write_imageui(histoPyramid, pos, (uint4)(nrOfTriangles[cubeindex], cubeindex, first, 0));
}*/

__kernel void classifyCubes2D(
        __write_only image2d_t histoPyramid, 
        __read_only image2d_t rawData,
        __private unsigned int res,
        __private unsigned int slices,
        __private float isolevel
        ) {
    int4 p4 = {get_global_id(0), get_global_id(1), get_global_id(2), 0};
    int2 pos = map3Dto2D(p4,res,slices);
    //int2 pos2 = map3Dto2D(p4+(int4)(0,0,1,0),res,slices);


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
//#endif
