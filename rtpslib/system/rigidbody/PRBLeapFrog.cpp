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

#include "PRBLeapFrog.h"

namespace rtps
{
    PRBLeapFrog::PRBLeapFrog(std::string path, CL* cli_, EB::Timer* timer_)
    {
        cli = cli_;
        timer = timer_;

        printf("create leapfrog kernel\n");
        path += "/leapfrog.cl";
        k_leapfrog = Kernel(cli, path, "leapfrog");

    }
    void PRBLeapFrog::execute(int num,
                    float dt,
                    Buffer<float4>& comLinearForce,
                    Buffer<float4>& comTorqueForce,
                    Buffer<float4>& comVel,
                    Buffer<float4>& comAngVel,
                    Buffer<float4>& comVelEval,
                    Buffer<float4>& comAngVelEval,
                    Buffer<float4>& comPos,
                    Buffer<float4>& comRot,
                    Buffer<float16>& inertialTensor,
                    Buffer<float>& rbMass,
                    int numRBs,
                    Buffer<ParticleRigidBodyParams>& prbp,
                    //debug params
                    Buffer<float4>& clf_debug,
                    Buffer<int4>& cli_debug)
    {

        int iargs = 0;
        k_leapfrog.setArg(iargs++, comLinearForce.getDevicePtr());
        k_leapfrog.setArg(iargs++, comTorqueForce.getDevicePtr());
        k_leapfrog.setArg(iargs++, comVel.getDevicePtr());
        k_leapfrog.setArg(iargs++, comAngVel.getDevicePtr());
        k_leapfrog.setArg(iargs++, comVelEval.getDevicePtr());
        k_leapfrog.setArg(iargs++, comAngVelEval.getDevicePtr());
        k_leapfrog.setArg(iargs++, comPos.getDevicePtr());
        k_leapfrog.setArg(iargs++, comRot.getDevicePtr());
        k_leapfrog.setArg(iargs++, inertialTensor.getDevicePtr());
        k_leapfrog.setArg(iargs++, rbMass.getDevicePtr());
        k_leapfrog.setArg(iargs++, dt); //time step
        k_leapfrog.setArg(iargs++, prbp.getDevicePtr());
        k_leapfrog.setArg(iargs++, clf_debug.getDevicePtr());
        k_leapfrog.setArg(iargs++, cli_debug.getDevicePtr());

        int local_size = 128;
        float gputime = k_leapfrog.execute(numRBs, local_size);
        if(gputime > 0)
            timer->set(gputime);


    }


#if 0
#define DENS 0
#define POS 1
#define VEL 2

        printf("************ LeapFrog **************\n");
            int nbc = num+5;
            std::vector<float4> poss(nbc);
            std::vector<float4> uposs(nbc);
            std::vector<float4> dens(nbc);

            //cl_vars_sorted.copyToHost(dens, DENS*sphp.max_num);
            cl_vars_sorted.copyToHost(poss, POS*sphp.max_num);
            cl_vars_unsorted.copyToHost(uposs, POS*sphp.max_num);

            for (int i=0; i < nbc; i++)
            //for (int i=0; i < 10; i++)
            {
                poss[i] = poss[i] / sphp.simulation_scale;
                //printf("-----\n");
                //printf("clf_debug: %f, %f, %f, %f\n", clf[i].x, clf[i].y, clf[i].z, clf[i].w);
                printf("pos sorted: %f, %f, %f, %f\n", poss[i].x, poss[i].y, poss[i].z, poss[i].w);
                printf("pos unsorted: %f, %f, %f, %f\n", uposs[i].x, uposs[i].y, uposs[i].z, uposs[i].w);
                //printf("dens sorted: %f, %f, %f, %f\n", dens[i].x, dens[i].y, dens[i].z, dens[i].w);
            }

#endif

        /*
         * enables us to cut off after a couple iterations
         * by setting cut = 1 from some other function
        if(cut >= 1)
        {
            if (cut == 2) {exit(0);}
            cut++;
        }
        */



} //namespace rtps
