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

#include "PRBEuler.h"

namespace rtps
{
    PRBEuler::PRBEuler(std::string path, CL* cli_, EB::Timer* timer_)
    {
        cli = cli_;
        timer = timer_;
 
        printf("create euler kernel\n");
        path += "/euler.cl";
        k_euler = Kernel(cli, path, "euler");
    } 
    
    void PRBEuler::execute(int num,
                    float dt,
                    Buffer<float4>& pos_u,
                    Buffer<float4>& pos_s,
                    Buffer<float4>& vel_u,
                    Buffer<float4>& vel_s,
                    Buffer<float4>& linear_force_s,
                    Buffer<float4>& torque_force_s,
                    Buffer<float4>& color_u,
                    Buffer<float4>& color_s,

                    //Buffer<float4>& uvars, 
                    //Buffer<float4>& svars, 
                    Buffer<unsigned int>& indices,
                    //params
                    Buffer<ParticleRigidBodyParams>& prbp,
                    //debug params
                    Buffer<float4>& clf_debug,
                    Buffer<int4>& cli_debug)
    {

        int iargs = 0;
        //k_euler.setArg(iargs++, uvars.getDevicePtr());
        //k_euler.setArg(iargs++, svars.getDevicePtr());
        k_euler.setArg(iargs++, pos_u.getDevicePtr());
        k_euler.setArg(iargs++, pos_s.getDevicePtr());
        k_euler.setArg(iargs++, vel_u.getDevicePtr());
        k_euler.setArg(iargs++, vel_s.getDevicePtr());
        k_euler.setArg(iargs++, linear_force_s.getDevicePtr());
        k_euler.setArg(iargs++, torque_force_s.getDevicePtr());
        k_euler.setArg(iargs++, color_u.getDevicePtr());
        k_euler.setArg(iargs++, color_s.getDevicePtr());
        k_euler.setArg(iargs++, indices.getDevicePtr());
        k_euler.setArg(iargs++, prbp.getDevicePtr());
        k_euler.setArg(iargs++, dt); //time step

        int local_size = 128;
        k_euler.execute(num, local_size);

    }
}
