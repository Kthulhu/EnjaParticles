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
                    /*Buffer<float4>& pos_u,
                    Buffer<float4>& pos_s,
                    Buffer<float4>& vel_u,
                    Buffer<float4>& vel_s,
                    Buffer<float4>& linear_force_s,
                    Buffer<float4>& torque_force_s,*/
                    Buffer<float4>& comLinearForce,
                    Buffer<float4>& comTorqueForce,
                    Buffer<float4>& comVel,
                    Buffer<float4>& comAngVel,
                    Buffer<float4>& comPos,
                    Buffer<float4>& comRot,
                    Buffer<float16>& inertialTensor,
                    Buffer<float>& rbMass,
            float4 gravity,
                    int numRBs,
                    Buffer<ParticleRigidBodyParams>& prbp,
                    //debug params
                    Buffer<float4>& clf_debug,
                    Buffer<int4>& cli_debug)
    {

        int iargs = 0;
        //k_euler.setArg(iargs++, uvars.getDevicePtr());
        //k_euler.setArg(iargs++, svars.getDevicePtr());
        /*k_euler.setArg(iargs++, pos_u.getDevicePtr());
        k_euler.setArg(iargs++, pos_s.getDevicePtr());
        k_euler.setArg(iargs++, vel_u.getDevicePtr());
        k_euler.setArg(iargs++, vel_s.getDevicePtr());
        k_euler.setArg(iargs++, linear_force_s.getDevicePtr());
        k_euler.setArg(iargs++, torque_force_s.getDevicePtr());*/
        k_euler.setArg(iargs++,comLinearForce.getDevicePtr());
        k_euler.setArg(iargs++,comTorqueForce.getDevicePtr());
        k_euler.setArg(iargs++,comVel.getDevicePtr());
        k_euler.setArg(iargs++,comAngVel.getDevicePtr());
        k_euler.setArg(iargs++,comPos.getDevicePtr());
        k_euler.setArg(iargs++,comRot.getDevicePtr());
        k_euler.setArg(iargs++,inertialTensor.getDevicePtr());
        k_euler.setArg(iargs++,rbMass.getDevicePtr());
        k_euler.setArg(iargs++, gravity);
        k_euler.setArg(iargs++, dt); //time step
        k_euler.setArg(iargs++, prbp.getDevicePtr());
        k_euler.setArg(iargs++, clf_debug.getDevicePtr());
        k_euler.setArg(iargs++, cli_debug.getDevicePtr());

        //int local_size = 128;
        k_euler.execute(numRBs);//, local_size);

#if 1 //printouts    
        //DEBUGING
        
        if(numRBs > 0)// && choice == 0)
        {
            printf("============================================\n");
            printf("***** PRINT euler output ******\n");
            printf("num %d\n", numRBs);

            std::vector<int4> cli(numRBs);
            std::vector<float4> clf(numRBs);
            
            cli_debug.copyToHost(cli);
            clf_debug.copyToHost(clf);

            for (int i=0; i < numRBs; i++)
            //for (int i=0; i < 10; i++) 
            {
                //printf("-----\n");
                printf("clf_debug: %08f, %08f, %08f, %08f\n", clf[i].x, clf[i].y, clf[i].z, clf[i].w);
                //if(clf[i].w == 0.0) exit(0);
                //printf("cli_debug: %d, %d, %d, %d\n", cli[i].x, cli[i].y, cli[i].z, cli[i].w);
                //		printf("pos : %f, %f, %f, %f\n", pos[i].x, pos[i].y, pos[i].z, pos[i].w);
            }
        }
#endif

    }
}
