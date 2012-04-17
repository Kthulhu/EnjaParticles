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


#ifndef _CONSTRUCTHP_CL_
#define _CONSTRUCTHP_CL_ 
#include "indexmap.h"
__kernel void constructHPLevel2D(
        __write_only image2d_t writeHistoPyramid,
        __read_only image2d_t readHistoPyramid
    ) 
{
    const int2 coord = {get_global_id(0), get_global_id(1)};
    const int2 readPos = coord*2;
    float writeValue = read_imagef(readHistoPyramid, sampler, readPos).x + 
        read_imagef(readHistoPyramid, sampler, readPos+squareOffsets[1]).x + 
        read_imagef(readHistoPyramid, sampler, readPos+squareOffsets[2]).x + 
        read_imagef(readHistoPyramid, sampler, readPos+squareOffsets[3]).x; 
    write_imagef(writeHistoPyramid, coord, writeValue);
}
#endif