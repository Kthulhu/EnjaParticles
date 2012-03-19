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
    ColorField::ColorField(std::string path, CL* cli_, EB::Timer* timer_)
    {
        cli = cli_;
        timer = timer_;

        printf("load force\n");

        try
        {
            path = path + "/colorfield.cl";
            k_colorfield = Kernel(cli, path, "force_update");
        }
        catch (cl::Error er)
        {
            printf("ERROR(ColorField): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
        }


    }
    //----------------------------------------------------------------------

    void ColorField::execute(int num,
                    Buffer<float4>& pos_s,
                    Buffer<float>& dens_s,
                    Buffer<float4>& cfieldTex,
                    int res,
                    Buffer<unsigned int>& ci_start,
                    Buffer<unsigned int>& ci_end,
                    //params
                    Buffer<SPHParams>& sphp,
                    Buffer<GridParams>& gp,
                    //debug params
                    Buffer<float4>& clf_debug,
                    Buffer<int4>& cli_debug)
    {
        int iarg = 0;
        k_colorfield.setArg(iarg++, pos_s.getDevicePtr());
        k_colorfield.setArg(iarg++, dens_s.getDevicePtr());
        k_colorfield.setArg(iarg++, cfieldTex.getDevicePtr());
        k_colorfield.setArg(iarg++, ci_start.getDevicePtr());
        k_colorfield.setArg(iarg++, ci_end.getDevicePtr());
        k_colorfield.setArg(iarg++, gp.getDevicePtr());
        k_colorfield.setArg(iarg++, sphp.getDevicePtr());

        // ONLY IF DEBUGGING
        k_colorfield.setArg(iarg++, clf_debug.getDevicePtr());
        k_colorfield.setArg(iarg++, cli_debug.getDevicePtr());

        int local = 64;
        try
        {
            float gputime = k_colorfield.execute(cl::NDRange(res,res,res));
            if(gputime > 0)
                timer->set(gputime);

        }

        catch (cl::Error er)
        {
            printf("ERROR(force ): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
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

