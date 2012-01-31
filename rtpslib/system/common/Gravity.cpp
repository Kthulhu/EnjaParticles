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


#include "Gravity.h"

#include <string>

namespace rtps
{

	//----------------------------------------------------------------------
    Gravity::Gravity(std::string path, CL* cli_)
    {
        cli = cli_;
        //printf("create gravity kernel\n");
        path = path + "/gravity.cl";
        k_gravity = Kernel(cli, path, "gravity");
        k_gravityForce = Kernel(cli, path, "gravityForce");
    }

    void Gravity::execute(int numPart,
                    int numPoint,
                    Buffer<float4>& pointSources,
                    Buffer<float>& massSources,
                    Buffer<float>& alphaSources,
                    Buffer<float4>& pos,
                    Buffer<float4>& accel,
                    float scale)
    {

        
        /*std::vector<float4> fvec(sampSize);
        std::fill(fvec.begin(), fvec.end(), float4(0.0,0.0,0.0,0.0));

        std::vector<int4> uivec(sampSize);
        std::fill(uivec.begin(), uivec.end(), int4(0,0,0,0));
        Buffer<float4> debugf(cli,fvec);
        Buffer<int4> debugi(cli,uivec);*/
        int iarg = 0;
        k_gravity.setArg(iarg++, numPart);
        k_gravity.setArg(iarg++, numPoint);
        k_gravity.setArg(iarg++, pointSources.getDevicePtr());
        k_gravity.setArg(iarg++, massSources.getDevicePtr());
        k_gravity.setArg(iarg++, alphaSources.getDevicePtr());
        k_gravity.setArg(iarg++, pos.getDevicePtr());
        k_gravity.setArg(iarg++, accel.getDevicePtr());
        k_gravity.setArg(iarg++, scale);
        

        try
        {
            float gputime = k_gravity.execute(numPart);
        }
        catch (cl::Error er)
        {
            printf("ERROR(data structures): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
        }
    }
    void Gravity::execute(int numPart,
                    int numPoint,
                    Buffer<float4>& pointSources,
                    Buffer<float>& massSources,
                    Buffer<float>& alphaSources,
                    Buffer<float4>& pos,
                    Buffer<float>& mass,
                    Buffer<float4>& force,
                    float scale)
    {

        
        /*std::vector<float4> fvec(sampSize);
        std::fill(fvec.begin(), fvec.end(), float4(0.0,0.0,0.0,0.0));

        std::vector<int4> uivec(sampSize);
        std::fill(uivec.begin(), uivec.end(), int4(0,0,0,0));
        Buffer<float4> debugf(cli,fvec);
        Buffer<int4> debugi(cli,uivec);*/
        int iarg = 0;
        k_gravityForce.setArg(iarg++, numPart);
        k_gravityForce.setArg(iarg++, numPoint);
        k_gravityForce.setArg(iarg++, pointSources.getDevicePtr());
        k_gravityForce.setArg(iarg++, massSources.getDevicePtr());
        k_gravityForce.setArg(iarg++, alphaSources.getDevicePtr());
        k_gravityForce.setArg(iarg++, pos.getDevicePtr());
        k_gravityForce.setArg(iarg++, mass.getDevicePtr());
        k_gravityForce.setArg(iarg++, force.getDevicePtr());
        k_gravityForce.setArg(iarg++, scale);
        

        try
        {
            float gputime = k_gravityForce.execute(numPart);
        }
        catch (cl::Error er)
        {
            printf("ERROR(data structures): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
        }
    }
	//----------------------------------------------------------------------
}
