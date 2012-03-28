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


#ifndef RTPS_MARCHINGCUBES_H_INCLUDED
#define RTPS_MARCHINGCUBES_H_INCLUDED

#include "../../opencl/CLL.h"
#include "../../opencl/Buffer.h"
#include "../../opencl/Kernel.h"
#include "../../render/MeshEffect.h"
#include "../../structs.h"

#include <vector>

namespace rtps
{
    class MarchingCubes
    {
        public:
            MarchingCubes() { cli = NULL; timer = NULL; };
            MarchingCubes(const MarchingCubes& mc);
            const MarchingCubes& operator=(const MarchingCubes& mc);
            MarchingCubes(std::string path, CL* cli, EB::Timer* timer,unsigned int res);
            void initializeData();
            struct Mesh* execute(cl::Image2D& colorfield,
                    unsigned int res,
                    //debug params
                    Buffer<float4>& clf_debug,
                    Buffer<int4>& cli_debug);

        private:
            void clone(const MarchingCubes& mc);
            cl::size_t<3> origin;
            cl::size_t<3> region;
            CL* cli;
            Kernel k_classify;
            Kernel k_construct;
            std::vector<Kernel> k_traverse;
            EB::Timer* timer;
            Buffer<float> cl_triangles;
            Buffer<float> cl_normals;
            struct Mesh mesh;
            std::vector<cl::Image2D> cl_histopyramid;
            unsigned int res;
            unsigned int levels;
            unsigned int texRes2D;
            unsigned int slices;

    };
}

#endif
