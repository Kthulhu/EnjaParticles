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


#ifndef _INDEXMAP_H_
#define _INDEXMAP_H_
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
int2 map3Dto2D(int4 coord, unsigned int res, unsigned int slices)
{
    const int yoffset = coord.z/slices;
    const int xoffset = coord.z%slices;
    
    int2 pos = {xoffset*res,yoffset*res};
    pos+=(int2)(coord.x,coord.y);
    return pos;
}

int2 map3Dto2DClamp(int4 coordinate, int4 offset, unsigned int res, unsigned int slices)
{
    int4 coord=coordinate+offset;
    if(coord.z<0||coord.z>res-1)
	coord.z-=offset.z;
    const int yoffset = coord.z/slices;
    const int xoffset = coord.z%slices;
    
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
}

int4 map2Dto3D(int2 coord, unsigned int res,unsigned int slices)
{
    const int z = (coord.y/res)*slices+(coord.x/res);
    return (int4)(coord.x%res,coord.y%res,z,0);
}

#endif
