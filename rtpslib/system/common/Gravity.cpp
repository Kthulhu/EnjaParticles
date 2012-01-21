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
        
    }

    void Gravity::execute(int numPart,
                    int numPoint,
                    Buffer<float4>& pointSources,
                    Buffer<float>& massSources,
                    float alpha,
                    Buffer<float4>& pos,
                    Buffer<float4>& accel)
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
        k_gravity.setArg(iarg++, alpha);
        k_gravity.setArg(iarg++, pos.getDevicePtr());
        k_gravity.setArg(iarg++, accel.getDevicePtr());
        

        try
        {
            float gputime = k_gravity.execute(numPart);
        }
        catch (cl::Error er)
        {
            printf("ERROR(data structures): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
        }

        
#if 0
        //printGravityDiagnostics();

        printf("**************** Gravity Diagnostics ****************\n");
        std::vector<float4> fl(5);
        std::vector<int4> in(5);
        
        debugf.copyToHost(fl);
        debugi.copyToHost(in);

        for(int i = 0; i < 5; i++)
        {
            printf("pos[%d] = (%f,%f,%f,%f)\n", i, fl[i].x, fl[i].y, fl[i].z, fl[i].w);
            printf("index[%d] = (%d,%d,%d,%d)\n", i, in[i].x, in[i].y, in[i].z, in[i].w);
        }

#endif

#if 0
        //print out elements from the sorted arrays
#define DENS 0
#define POS 1
#define VEL 2

			printf("gordon\n");
            int nbc = num+5;
            std::vector<float4> hpos_u(nbc);
            std::vector<float4> hpos_s(nbc);
            std::vector<unsigned int> hindices(nbc);

            //svars.copyToHost(dens, DENS*sphp.max_num);
            //svars.copyToHost(poss, POS*sphp.max_num);

			pos_u.copyToHost(hpos_u);
			pos_s.copyToHost(hpos_s);
			indices.copyToHost(hindices);

			printf("**** INSIDE PERMUTE ****\n");

			printf("**** UNSORTED POSITIONS *****\n");
            for (int i=0; i < num; i++)
            {
                //printf("clf_debug: %f, %f, %f, %f\n", clf[i].x, clf[i].y, clf[i].z, clf[i].w);
                printf("pos unsorted: %f, %f, %f, %f\n", hpos_u[i].x, hpos_u[i].y, hpos_u[i].z, hpos_u[i].w);
            }

			printf("**** SORTED POSITIONS *****\n");
            for (int i=0; i < num; i++)
            {
                printf("pos sorted: %f, %f, %f, %f\n", hpos_s[i].x, hpos_s[i].y, hpos_s[i].z, hpos_s[i].w);
            }

			printf("**** SORTED INDICES *****\n");
            for (int i=0; i < num; i++)
            {
                printf("indices: %d\n", hindices[i]);
            }
#endif



        //return nc;
    }
	//----------------------------------------------------------------------
}
