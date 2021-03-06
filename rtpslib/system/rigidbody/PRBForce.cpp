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

#include "PRBForce.h"

namespace rtps
{

    //----------------------------------------------------------------------
    PRBForce::PRBForce(std::string path, CL* cli_, EB::Timer* timer_)
    {
        cli = cli_;
        timer = timer_;

        printf("load force\n");

        try
        {
            path = path + "/force.cl";
            k_force = Kernel(cli, path, "force_update");
            //path = path + "/force_active.cl";
            //k_force_active = Kernel(cli, path, "force_update_active");
        }
        catch (cl::Error er)
        {
            printf("ERROR(Force): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
        }


    }
    //----------------------------------------------------------------------
    /*void PRBForce::executeActive(int num,
                    Buffer<float4>& pos_s,
                    Buffer<float4>& veleval_s,
                    Buffer<float4>& linear_force_s,
                    Buffer<float>& mass_s,
                    Buffer<unsigned int>& objectIndex_s,
                    //Buffer<float4>& torque_force_s,
                    Buffer<unsigned int>& indices,
                    Buffer<unsigned int>& ci_start,
                    Buffer<unsigned int>& ci_end,
                    //params
                    Buffer<ParticleRigidBodyParams>& prbp,
                    Buffer<GridParams>& gp,
                    Buffer<float4>& active_cells_s,
                    Buffer<float4>& active_lines_s,
                    Buffer<float4>& active_col_s,
                    Buffer<float4>& clf_debug,
                    Buffer<int4>& cli_debug
                    )
    {
        int iarg = 0;
        k_force.setArg(iarg++, pos_s.getDevicePtr());
        k_force.setArg(iarg++, veleval_s.getDevicePtr());
        k_force.setArg(iarg++, linear_force_s.getDevicePtr());
        k_force.setArg(iarg++, mass_s.getDevicePtr());
        k_force.setArg(iarg++, objectIndex_s.getDevicePtr());
        k_force.setArg(iarg++, active_cells_s.getDevicePtr());
        k_force.setArg(iarg++, active_lines_s.getDevicePtr());
        k_force.setArg(iarg++, active_col_s.getDevicePtr());
        //k_force.setArg(iarg++, torque_force_s.getDevicePtr());
        k_force.setArg(iarg++, indices.getDevicePtr());
        k_force.setArg(iarg++, ci_start.getDevicePtr());
        k_force.setArg(iarg++, ci_end.getDevicePtr());
        k_force.setArg(iarg++, gp.getDevicePtr());
        k_force.setArg(iarg++, prbp.getDevicePtr());

        // ONLY IF DEBUGGING
        k_force.setArg(iarg++, clf_debug.getDevicePtr());
        k_force.setArg(iarg++, cli_debug.getDevicePtr());
        int local = 64;
        try
        {
            float gputime = k_force.execute(num, local);
            if(gputime > 0)
                timer->set(gputime);

        }

        catch (cl::Error er)
        {
            printf("ERROR(force ): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
        }

#if 0 //printouts
        //DEBUGING

        num=10;
        if(num > 0)// && choice == 0)
        {
            printf("============================================\n");
            printf("***** PRINT Rigid Body Forces ******\n");
            printf("num %d\n", num);

            std::vector<int4> cli(num);
            std::vector<float4> clf(num);

            cli_debug.copyToHost(cli);
            clf_debug.copyToHost(clf);

            for (int i=0; i < num; i++)
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
*/

    void PRBForce::execute(int num,
                    Buffer<float4>& pos_s,
                    Buffer<float4>& veleval_s,
                    Buffer<float4>& linear_force_s,
                    Buffer<float>& mass_s,
                    Buffer<unsigned int>& objectIndex_s,
                    //Buffer<float4>& torque_force_s,
                    Buffer<unsigned int>& indices,
                    Buffer<unsigned int>& ci_start,
                    Buffer<unsigned int>& ci_end,
                    //params
                    Buffer<ParticleRigidBodyParams>& prbp,
                    Buffer<GridParams>& gp,
                    //debug params
                    Buffer<float4>& clf_debug,
                    Buffer<int4>& cli_debug)
    {
        int iarg = 0;
        k_force.setArg(iarg++, pos_s.getDevicePtr());
        k_force.setArg(iarg++, veleval_s.getDevicePtr());
        k_force.setArg(iarg++, linear_force_s.getDevicePtr());
        k_force.setArg(iarg++, mass_s.getDevicePtr());
        k_force.setArg(iarg++, objectIndex_s.getDevicePtr());
        //k_force.setArg(iarg++, torque_force_s.getDevicePtr());
        k_force.setArg(iarg++, indices.getDevicePtr());
        k_force.setArg(iarg++, ci_start.getDevicePtr());
        k_force.setArg(iarg++, ci_end.getDevicePtr());
        k_force.setArg(iarg++, gp.getDevicePtr());
        k_force.setArg(iarg++, prbp.getDevicePtr());

        // ONLY IF DEBUGGING
        k_force.setArg(iarg++, clf_debug.getDevicePtr());
        k_force.setArg(iarg++, cli_debug.getDevicePtr());

        int local = 64;
        try
        {
            float gputime = k_force.execute(num, local);
            if(gputime > 0)
                timer->set(gputime);

        }

        catch (cl::Error er)
        {
            printf("ERROR(force ): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
        }

#if 0 //printouts
        //DEBUGING

        num=10;
        if(num > 0)// && choice == 0)
        {
            printf("============================================\n");
            printf("***** PRINT Rigid Body Forces ******\n");
            printf("num %d\n", num);

            std::vector<int4> cli(num);
            std::vector<float4> clf(num);

            cli_debug.copyToHost(cli);
            clf_debug.copyToHost(clf);

            for (int i=0; i < num; i++)
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

