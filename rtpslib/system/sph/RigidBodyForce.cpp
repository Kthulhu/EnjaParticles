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


#include <SPH.h>

namespace rtps
{

    //----------------------------------------------------------------------
    RigidBodyForce::RigidBodyForce(std::string path, CL* cli_, EB::Timer* timer_)
    {
        cli = cli_;
        timer = timer_;

        printf("load rigidbody force\n");

        try
        {
            string tmp=path;
            path = path + "/rigidbody_force.cl";
            k_rigidbody_force = Kernel(cli, path, "force_update");
            path = tmp + "/staticrigidbody_force.cl";
            k_staticrigidbody_force = Kernel(cli, path, "force_update");
        }
        catch (cl::Error er)
        {
            printf("ERROR(RigidBodyForce): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
        }


    }
    //----------------------------------------------------------------------

    void RigidBodyForce::execute(int num,
                    Buffer<float4>& pos_s,
                    Buffer<float4>& veleval_s,
                    Buffer<float4>& force_s,
                    Buffer<float>& mass_s,
                    Buffer<float4>& rb_pos_s,
                    Buffer<float4>& rb_velocity_s,
                    Buffer<float>& rb_mass_s,
                    Buffer<unsigned int>& ci_start,
                    Buffer<unsigned int>& ci_end,
                    //params
                    Buffer<SPHParams>& sphp,
                    Buffer<GridParams>& gp,
                    float stiffness,
                    float dampening,
                    float friction_dynamic,
                    float friction_static,
                    float friction_static_threshold,
                    //debug params
                    Buffer<float4>& clf_debug,
                    Buffer<int4>& cli_debug)
    {
        int iarg = 0;
        k_rigidbody_force.setArg(iarg++, pos_s.getDevicePtr());
        k_rigidbody_force.setArg(iarg++, veleval_s.getDevicePtr());
        k_rigidbody_force.setArg(iarg++, force_s.getDevicePtr());
        k_rigidbody_force.setArg(iarg++, mass_s.getDevicePtr());
        k_rigidbody_force.setArg(iarg++, rb_pos_s.getDevicePtr());
        k_rigidbody_force.setArg(iarg++, rb_velocity_s.getDevicePtr());
        k_rigidbody_force.setArg(iarg++, rb_mass_s.getDevicePtr());
        float16 rbParams;
        rbParams.m[0]=stiffness;
        rbParams.m[1]=dampening;
        rbParams.m[2]=friction_dynamic;
        rbParams.m[3]=friction_static;
        rbParams.m[4]=friction_static_threshold;
        k_rigidbody_force.setArg(iarg++, rbParams);
        k_rigidbody_force.setArg(iarg++, ci_start.getDevicePtr());
        k_rigidbody_force.setArg(iarg++, ci_end.getDevicePtr());
        k_rigidbody_force.setArg(iarg++, gp.getDevicePtr());
        k_rigidbody_force.setArg(iarg++, sphp.getDevicePtr());

        // ONLY IF DEBUGGING
        k_rigidbody_force.setArg(iarg++, clf_debug.getDevicePtr());
        k_rigidbody_force.setArg(iarg++, cli_debug.getDevicePtr());

        int local = 64;
        try
        {
            float gputime = k_rigidbody_force.execute(num, local);
            if(gputime > 0)
                timer->set(gputime);

        }

        catch (cl::Error er)
        {
            printf("ERROR(rigidbody force ): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
        }

#if 0 //printouts
        //DEBUGING

        if(num > 0)// && choice == 0)
        {
            printf("============================================\n");
            printf("which == %d *** \n", choice);
            printf("***** PRINT neighbors diagnostics ******\n");
            printf("num %d\n", num);

            std::vector<int4> cli(num);
            std::vector<float4> clf(num);

            cli_debug.copyToHost(cli);
            clf_debug.copyToHost(clf);

            std::vector<float4> poss(num);
            std::vector<float4> dens(num);

            for (int i=0; i < num; i++)
            //for (int i=0; i < 10; i++)
            {
                //printf("-----\n");
                printf("clf_debug: %f, %f, %f, %f\n", clf[i].x, clf[i].y, clf[i].z, clf[i].w);
                //if(clf[i].w == 0.0) exit(0);
                //printf("cli_debug: %d, %d, %d, %d\n", cli[i].x, cli[i].y, cli[i].z, cli[i].w);
                //printf("pos : %f, %f, %f, %f\n", pos[i].x, pos[i].y, pos[i].z, pos[i].w);
            }
        }
#endif
    }
    void RigidBodyForce::execute(int num,
                    Buffer<float4>& pos_s,
                    Buffer<float4>& veleval_s,
                    Buffer<float4>& force_s,
                    Buffer<float>& mass_s,
                    Buffer<float4>& rb_pos_s,
                    Buffer<unsigned int>& ci_start,
                    Buffer<unsigned int>& ci_end,
                    //params
                    Buffer<SPHParams>& sphp,
                    Buffer<GridParams>& gp,
                    float stiffness,
                    float dampening,
                    float friction_dynamic,
                    float friction_static,
                    float friction_static_threshold,
                    //debug params
                    Buffer<float4>& clf_debug,
                    Buffer<int4>& cli_debug)
    {
        int iarg = 0;
        k_staticrigidbody_force.setArg(iarg++, pos_s.getDevicePtr());
        k_staticrigidbody_force.setArg(iarg++, veleval_s.getDevicePtr());
        k_staticrigidbody_force.setArg(iarg++, force_s.getDevicePtr());
        k_staticrigidbody_force.setArg(iarg++, mass_s.getDevicePtr());
        k_staticrigidbody_force.setArg(iarg++, rb_pos_s.getDevicePtr());
        float16 rbParams;
        rbParams.m[0]=stiffness;
        rbParams.m[1]=dampening;
        rbParams.m[2]=friction_dynamic;
        rbParams.m[3]=friction_static;
        rbParams.m[4]=friction_static_threshold;
        k_staticrigidbody_force.setArg(iarg++, rbParams);
        k_staticrigidbody_force.setArg(iarg++, ci_start.getDevicePtr());
        k_staticrigidbody_force.setArg(iarg++, ci_end.getDevicePtr());
        k_staticrigidbody_force.setArg(iarg++, gp.getDevicePtr());
        k_staticrigidbody_force.setArg(iarg++, sphp.getDevicePtr());

        // ONLY IF DEBUGGING
        k_staticrigidbody_force.setArg(iarg++, clf_debug.getDevicePtr());
        k_staticrigidbody_force.setArg(iarg++, cli_debug.getDevicePtr());

        int local = 64;
        try
        {
            float gputime = k_staticrigidbody_force.execute(num, local);
            if(gputime > 0)
                timer->set(gputime);

        }

        catch (cl::Error er)
        {
            printf("ERROR(rigidbody force ): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
        }

#if 0 //printouts
        //DEBUGING

        if(num > 0)// && choice == 0)
        {
            printf("============================================\n");
            printf("which == %d *** \n", choice);
            printf("***** PRINT neighbors diagnostics ******\n");
            printf("num %d\n", num);

            std::vector<int4> cli(num);
            std::vector<float4> clf(num);

            cli_debug.copyToHost(cli);
            clf_debug.copyToHost(clf);

            std::vector<float4> poss(num);
            std::vector<float4> dens(num);

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

