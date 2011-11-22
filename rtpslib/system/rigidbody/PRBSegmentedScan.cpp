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

#include "PRBSegmentedScan.h"

namespace rtps 
{

    //----------------------------------------------------------------------
    PRBSegmentedScan::PRBSegmentedScan(std::string path, CL* cli_, EB::Timer* timer_)
    {
        cli = cli_;
        timer = timer_;
     
        printf("load segmented scan\n");

        try
        {
            path = path + "/segmented_scan.cl";
            k_segmented_scan = Kernel(cli, path, "sum");
        }
        catch (cl::Error er)
        {
            printf("ERROR(PRBSegmentedScan): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
        }


    }
    //----------------------------------------------------------------------

    void PRBSegmentedScan::execute(int num,
                    Buffer<float4>& pos_u,
                    //Buffer<float4>& veleval_s,
                    Buffer<int2>& particleIndex,
                    Buffer<float4>& linear_force_u,
                    Buffer<float4>& comLinearForce,
                    Buffer<float4>& comTorqueForce,
                    Buffer<float4>& comPos,
                    int numRBs,
                    //Buffer<float4>& torque_force_s,
                    //Buffer<unsigned int>& ci_start,
                    //Buffer<unsigned int>& ci_end,
                    //debug params
                    Buffer<float4>& clf_debug,
                    Buffer<int4>& cli_debug)
    { 
        int iarg = 0;
        k_segmented_scan.setArg(iarg++, pos_u.getDevicePtr());
        k_segmented_scan.setArg(iarg++, particleIndex.getDevicePtr());
        //k_segmented_scan.setArg(iarg++, veleval_s.getDevicePtr());
        k_segmented_scan.setArg(iarg++, linear_force_u.getDevicePtr());
        k_segmented_scan.setArg(iarg++, comLinearForce.getDevicePtr());
        k_segmented_scan.setArg(iarg++, comTorqueForce.getDevicePtr());
        k_segmented_scan.setArg(iarg++, comPos.getDevicePtr());
        //k_segmented_scan.setArg(iarg++, torque_force_s.getDevicePtr());
        //k_segmented_scan.setArg(iarg++, ci_start.getDevicePtr());
        //k_segmented_scan.setArg(iarg++, ci_end.getDevicePtr());
        // ONLY IF DEBUGGING
        k_segmented_scan.setArg(iarg++, clf_debug.getDevicePtr());
        k_segmented_scan.setArg(iarg++, cli_debug.getDevicePtr());

        int local = 64;
        try
        {
            float gputime = k_segmented_scan.execute(numRBs);//, local);
            if(gputime > 0)
                timer->set(gputime);

        }

        catch (cl::Error er)
        {
            printf("ERROR(PRBSegmentedScan): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
        }

#if 0 //printouts    
        //DEBUGING
        
        if(numRBs > 0)// && choice == 0)
        {
            printf("============================================\n");
            printf("***** PRINT Segmented Scan diagnostics ******\n");
            printf("numRBs %d\n", numRBs);

            std::vector<int4> cli(numRBs);
            std::vector<float4> clf(numRBs);
            
            cli_debug.copyToHost(cli);
            clf_debug.copyToHost(clf);

            for (int i=0; i < numRBs; i++)
            //for (int i=0; i < 10; i++) 
            {
                //printf("-----\n");
                printf("clf_debug: %f, %f, %f, %f\n", clf[i].x, clf[i].y, clf[i].z, clf[i].w);
                //if(clf[i].w == 0.0) exit(0);
                //printf("cli_debug: %d, %d, %d, %d\n", cli[i].x, cli[i].y, cli[i].z, cli[i].w);
                //		printf("pos : %f, %f, %f, %f\n", pos[i].x, pos[i].y, pos[i].z, pos[i].w);
            }
        }
#endif
    }


}

