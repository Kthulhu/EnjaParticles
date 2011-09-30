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


#include "MeshToParticles.h"

#include <string>

namespace rtps
{

    MeshToParticles::MeshToParticles(std::string path, CL* cli_, EB::Timer* timer_)
    {
        cli = cli_;
        timer = timer_;
        //printf("create meshtoparticles kernel\n");
        path = path + "/meshtoparticles.cl";
        k_meshtoparticles = Kernel(cli, path, "meshToParticles");
        
    }

    int MeshToParticles::execute(
                    Buffer<float4>& pos,
                    Buffer<float4>& color,
                    Buffer<float4>& velocity,
                    int num,
                    cl::Image3D posTex,
                    float4 extent,
                    float4 min,
                    float16 world,
                    int res,
                    //debug params
                    Buffer<float4>& clf_debug,
                    Buffer<int4>& cli_debug)
    {

		//printf("*** enter MeshToParticles, num= %d\n", num);

        std::vector<int> numVec;
        numVec.push_back(0);
        Buffer<int> newNum(cli,numVec);
        int iarg = 0;
        k_meshtoparticles.setArg(iarg++, pos.getDevicePtr());
        k_meshtoparticles.setArg(iarg++, color.getDevicePtr());
        k_meshtoparticles.setArg(iarg++, velocity.getDevicePtr());
        k_meshtoparticles.setArg(iarg++, num);
        k_meshtoparticles.setArg(iarg++, posTex);
        k_meshtoparticles.setArg(iarg++, extent);
        k_meshtoparticles.setArg(iarg++, min);
        k_meshtoparticles.setArg(iarg++, world);
        k_meshtoparticles.setArg(iarg++, res);
        k_meshtoparticles.setArg(iarg++, newNum.getDevicePtr());
        printf("extent (%f,%f,%f)\n",extent.x,extent.y,extent.z);
        printf("min (%f,%f,%f)\n",min.x,min.y,min.z);
        printf("world | %f\t%f\t%f\t%f |\n",world.m[0],world.m[1],world.m[2],world.m[3]);
        printf("world | %f\t%f\t%f\t%f |\n",world.m[4],world.m[5],world.m[6],world.m[7]);
        printf("world | %f\t%f\t%f\t%f |\n",world.m[8],world.m[9],world.m[10],world.m[11]);
        printf("world | %f\t%f\t%f\t%f |\n",world.m[12],world.m[13],world.m[14],world.m[15]);


        //printf("about to data structures\n");
        try
        {
            float gputime = k_meshtoparticles.execute(cl::NDRange(res,res,res));
            if(gputime > 0)
                timer->set(gputime);
            newNum.copyToHost(numVec);
            
        }
        catch (cl::Error er)
        {
            printf("ERROR(meshtoparticles): %s(%s)\n", er.what(), oclErrorString(er.err()));
        }

#if 0
        //printMeshToParticlesDiagnostics();

        printf("**************** MeshToParticles Diagnostics ****************\n");
        int nbc = nb_cells + 1;
        printf("nb_cells: %d\n", nbc); // nb grid cells?
        printf("num: %d\n", num); // nb grid cells?
        printf("cell indices, num particles: %d\n", num);

        std::vector<unsigned int> is(nbc);
        std::vector<unsigned int> ie(nbc);
        std::vector<unsigned int> in(nbc);
        std::vector<unsigned int> hhash(nbc);
        
        ci_end.copyToHost(ie);
        ci_start.copyToHost(is);
        indices.copyToHost(in);
        hashes.copyToHost(hhash);

        for(int i = 0; i < nbc; i++)
        {
            if (is[i] != -1)// && ie[i] != 0) // GE inserted comment
            {
                //nb = ie[i] - is[i];
                //nb_particles += nb;
                //if(is[i] < 8000 || ie[i] > 0) // GE put the comment
                {
					// in particle list
                    printf("cell: %d indices start: %d indices stop: %d\n", i, is[i], ie[i]);
					// hash is for cells (different number)
                    //printf("cell: %d hash: %d, index: %d\n", i, hhash[i], in[i]);
                }
            }
        }

#endif

#if 0
        //print out elements from the sorted arrays
#define DENS 0
#define POS 1
#define VEL 2

            nbc = num+5;
            std::vector<float4> poss(nbc);
            std::vector<float4> dens(nbc);

            //svars.copyToHost(dens, DENS*sphp.max_num);
            //svars.copyToHost(poss, POS*sphp.max_num);

            //for (int i=0; i < nbc; i++)
            for (int i=0; i < 10; i++) 
            {
                poss[i] = poss[i] / sphp.simulation_scale;
                //printf("-----\n");
                //printf("clf_debug: %f, %f, %f, %f\n", clf[i].x, clf[i].y, clf[i].z, clf[i].w);
                printf("pos sorted: %f, %f, %f, %f\n", poss[i].x, poss[i].y, poss[i].z, poss[i].w);
                //printf("dens sorted: %f, %f, %f, %f\n", dens[i].x, dens[i].y, dens[i].z, dens[i].w);
            }

#endif
            return numVec[0];
    }

}
