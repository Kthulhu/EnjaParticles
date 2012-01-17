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


#include "Sample.h"

#include <string>

namespace rtps
{

	//----------------------------------------------------------------------
    Sample::Sample(std::string path, CL* cli_)
    {
        cli = cli_;
        //printf("create sample kernel\n");
        path = path + "/sample.cl";
        k_sample = Kernel(cli, path, "sample");
        
    }

    void Sample::execute(int num,
                    Buffer<float4>& population,
                    Buffer<float4>& samples,
                    Buffer<unsigned int>& sampleIndices,
                    unsigned int insOffset,
                    unsigned int insStride)
    {

        
        int iarg = 0;
        k_sample.setArg(iarg++, num);
        k_sample.setArg(iarg++, population.getDevicePtr());
        k_sample.setArg(iarg++, samples.getDevicePtr());
        k_sample.setArg(iarg++, sampleIndices.getDevicePtr());
        k_sample.setArg(iarg++, insOffset);
        k_sample.setArg(iarg++, insStride);

        try
        {
			//printf("k_sample (non-cloud): num= %d\n", num); 
            float gputime = k_sample.execute(num);
        }
        catch (cl::Error er)
        {
            printf("ERROR(data structures): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
        }

        
#if 0
        //printSampleDiagnostics();

        printf("**************** Sample Diagnostics ****************\n");
        int nbc = nb_cells + 1;
        printf("nb_cells: %d\n", nbc);
        printf("num particles: %d\n", num);

        std::vector<unsigned int> is(nbc);
        std::vector<unsigned int> ie(nbc);
        
        ci_end.copyToHost(ie);
        ci_start.copyToHost(is);

        //std::vector<unsigned int> hpos_u(nbc);
        //std::vector<unsigned int> hpos_s(nbc);
		//pos_s.copyToHost(hpos_s);
		//pos_u.copyToHost(hpos_u);

        for(int i = 0; i < nbc; i++)
        {
            if (is[i] != -1)// && ie[i] != 0)
            {
                //nb = ie[i] - is[i];
                //nb_particles += nb;
                printf("cell: %d indices start: %d indices stop: %d\n", i, is[i], ie[i]);
            }
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
