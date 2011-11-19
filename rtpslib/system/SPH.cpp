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



#include <GL/glew.h>
#include <math.h>
#include <sstream>
#include <iomanip>
#include <string>

#include "System.h"
#include "SPH.h"
//#include "../domain/UniformGrid.h"
#include "Domain.h"
#include "IV.h"

#include "common/Hose.h"

//for random
#include<time.h>

namespace rtps
{
    using namespace sph;

	//----------------------------------------------------------------------
    SPH::SPH(RTPS *psfr, int n, int nb_in_cloud=0):System(psfr,n)
    {
        nb_var = 10;

        std::vector<SPHParams> vparams(0);
        vparams.push_back(sphp);
        cl_sphp = Buffer<SPHParams>(ps->cli, vparams);

        calculate();
        updateParams();

        //settings->printSettings();

        spacing = settings->GetSettingAs<float>("Spacing");

        //SPH settings depend on number of particles used
        //calculateSPHSettings();
        //set up the grid
        setupDomain(sphp.smoothing_distance/sphp.simulation_scale,sphp.simulation_scale);

        integrator = LEAPFROG;
        //integrator = EULER;

        setupTimers();

#ifdef GPU
        printf("RUNNING ON THE GPU\n");
        prepareSorted();

        //should be more cross platform
        sph_source_dir = resource_path + "/" + std::string(SPH_CL_SOURCE_DIR);
        ps->cli->addIncludeDir(sph_source_dir);

        forceRB = RigidBodyForce(sph_source_dir, ps->cli, timers["force_rigidbody_gpu"]);
        density = Density(sph_source_dir, ps->cli, timers["density_gpu"]);
        force = Force(sph_source_dir, ps->cli, timers["force_gpu"]);
        collision_wall = CollisionWall(sph_source_dir, ps->cli, timers["cw_gpu"]);
        collision_tri = CollisionTriangle(sph_source_dir, ps->cli, timers["ct_gpu"], 2048); //TODO expose max_triangles as a parameter
		

        //could generalize this to other integration methods later (leap frog, RK4)
        if (integrator == LEAPFROG)
        {
            //loadLeapFrog();
            leapfrog = LeapFrog(sph_source_dir, ps->cli, timers["leapfrog_gpu"]);
        }
        else if (integrator == EULER)
        {
            //loadEuler();
            euler = Euler(sph_source_dir, ps->cli, timers["euler_gpu"]);
        }
#endif
        renderer->setParticleRadius(spacing);
    }

	//----------------------------------------------------------------------
    SPH::~SPH()
    {
        printf("SPH destructor\n");

        Hose* hose;
        int hs = hoses.size();  
        for(int i = 0; i < hs; i++)
        {
            hose = hoses[i];
            delete hose;

        }
    }

	//----------------------------------------------------------------------
    void SPH::update()
    {
		//printf("+++++++++++++ enter UPDATE()\n");
        //call kernels
        //TODO: add timings
#ifdef CPU
        updateCPU();
#endif
#ifdef GPU
        updateGPU();
#endif
    }

	//----------------------------------------------------------------------
    /*void SPH::updateCPU()
    {
        cpuDensity();
        cpuPressure();
        cpuViscosity();
        cpuXSPH();
        cpuCollision_wall();

        if (integrator == EULER)
        {
            cpuEuler();
        }
        else if (integrator == LEAPFROG)
        {
            cpuLeapFrog();
        }
        glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
        glBufferData(GL_ARRAY_BUFFER, num * sizeof(float4), &positions[0], GL_DYNAMIC_DRAW);
    }*/

	//----------------------------------------------------------------------
    void SPH::updateGPU()
    {
		//printf("**** enter updateGPU, num= %d\n", num);

        timers["update"]->start();
        if (settings->has_changed()) updateParams();

        //settings->printSettings();

        //int sub_intervals = 3;  //should be a setting
        int sub_intervals =  settings->GetSettingAs<float>("sub_intervals");
        //this should go in the loop but avoiding acquiring and releasing each sub
        //interval for all the other calls.
        //this does end up acquire/release everytime sprayHoses calls pushparticles
        //should just do try/except?
        for (int i=0; i < sub_intervals; i++)
        {
            sprayHoses();
        }

        for (int i=0; i < sub_intervals; i++)
        {
            //if(num >0) printf("before hash and sort\n");
            hash_and_sort();

			//------------------
            //printf("before cellindices, num= %d\n", num);
            timers["cellindices"]->start();
            int nc = cellindices.execute(   num,
                cl_sort_hashes,
                cl_sort_indices,
                cl_cell_indices_start,
                cl_cell_indices_end,
                //cl_sphp,
                cl_GridParams,
                grid_params.nb_cells,
                clf_debug,
                cli_debug);
            timers["cellindices"]->stop();

			//if (num > 0) exit(1); //GE

       
			//-----------------
            //printf("*** enter fluid permute, num= %d\n", num);
            timers["permute"]->start();
            permute.execute(   num,
                cl_position_u,
                cl_position_s,
                cl_velocity_u,
                cl_velocity_s,
                cl_veleval_u,
                cl_veleval_s,
                cl_color_u,
                cl_color_s,
                cl_sort_indices,
                //cl_sphp,
                cl_GridParams,
                clf_debug,
                cli_debug);
            timers["permute"]->stop();
			//printf("exit after fluid permute\n");
			//if (num > 0) exit(0);
 
			// NUMBER OF CLOUD PARTICLES IS CONSTANT THROUGHOUT THE SIMULATION

			//---------------------
            if (nc <= num && nc >= 0)
            {
                //check if the number of particles has changed
                //(this happens when particles go out of bounds,
                //  either because of forces or by explicitly placing
                //  them in order to delete)
                //
                //if so we need to copy sorted into unsorted
                //and redo hash_and_sort
                printf("SOME PARTICLES WERE DELETED!\n");
                printf("nc: %d num: %d\n", nc, num);

                deleted_pos.resize(num-nc);
                deleted_vel.resize(num-nc);
                //The deleted particles should be the nc particles after num
                cl_position_s.copyToHost(deleted_pos, nc); //damn these will always be out of bounds here!
                cl_velocity_s.copyToHost(deleted_vel, nc);

 
                num = nc;
                settings->SetSetting("Number of Particles", num);
                //sphp.num = num;
                updateParams();
                renderer->setNum(sphp.num);

                //need to copy sorted arrays into unsorted arrays
//**** PREP(2)
                call_prep(2);
                //printf("HOW MANY NOW? %d\n", num);
                hash_and_sort();
                                //we've changed num and copied sorted to unsorted. skip this iteration and do next one
                //this doesn't work because sorted force etc. are having an effect?
                //continue; 
            }


			//-------------------------------------
            //if(num >0) printf("density\n");
            timers["density"]->start();
            density.execute(   num,
                //cl_vars_sorted,
                cl_position_s,
                cl_density_s,
                cl_cell_indices_start,
                cl_cell_indices_end,
                cl_sphp,
                cl_GridParamsScaled, // GE: Might have to fix this. Do not know. 
                //cl_GridParams,
                clf_debug,
                cli_debug);
            timers["density"]->stop();
            
			//-------------------------------------
            //if(num >0) printf("force\n");
            timers["force"]->start();
            force.execute(   num,
                //cl_vars_sorted,
                cl_position_s,
                cl_density_s,
                cl_veleval_s,
                cl_force_s,
                cl_xsph_s,
                cl_cell_indices_start,
                cl_cell_indices_end,
                cl_sphp,
                //cl_GridParams,
                cl_GridParamsScaled,
                clf_debug,
                cli_debug);

            timers["force"]->stop();

            collision();

        }


        timers["update"]->stop();
    }

	//----------------------------------------------------------------------
    void SPH::collision()
    {
        //when implemented other collision routines can be chosen here
        timers["collision_wall"]->start();
        //collide_wall();
        collision_wall.execute(num,
                //cl_vars_sorted, 
                cl_position_s,
                cl_velocity_s,
                cl_force_s,
                cl_sphp,
                //cl_GridParams,
                cl_GridParamsScaled,
                //debug
                clf_debug,
                cli_debug);

        //k_collision_wall.execute(num, local_size);
        timers["collision_wall"]->stop();

        timers["collision_tri"]->start();
        //collide_triangles();
        collision_tri.execute(num,
                settings->dt,
                //cl_vars_sorted, 
                cl_position_s,
                cl_velocity_s,
                cl_force_s,
                cl_sphp,
                //debug
                clf_debug,
                cli_debug);
        timers["collision_tri"]->stop();
    }
	//----------------------------------------------------------------------

    void SPH::integrate()
    {
        timers["integrate"]->start();
        if (integrator == EULER)
        {
            //euler();
            euler.execute(num,
                settings->dt,
                cl_position_u,
                cl_position_s,
                cl_velocity_u,
                cl_velocity_s,
                cl_force_s,
                //cl_vars_unsorted, 
                //cl_vars_sorted, 
                cl_sort_indices,
                cl_sphp,
                //debug
                clf_debug,
                cli_debug);


        }
        else if (integrator == LEAPFROG)
        {
            //leapfrog();
             leapfrog.execute(num,
                settings->dt,
                cl_position_u,
                cl_position_s,
                cl_velocity_u,
                cl_velocity_s,
                cl_veleval_u,
                cl_force_s,
                cl_xsph_s,
                cl_color_u,
                cl_color_s,
                //cl_vars_unsorted, 
                //cl_vars_sorted, 
                cl_sort_indices,
                cl_sphp,
                //debug
                clf_debug,
                cli_debug);
        }

		// Perhaps I am messed up by Courant condition if cloud point 
		// velocities are too large? 

		static int count=0;

    	timers["integrate"]->stop();
    }

	// GE: WHY IS THIS NEEDED?
	//----------------------------------------------------------------------
    void SPH::call_prep(int stage)
    {
		// copy from sorted to unsorted arrays at the beginning of each 
		// iteration
		// copy from cl_position_s to cl_position_u
		// Only called if number of fluid particles changes from one iteration
		// to the other

            cl_position_u.copyFromBuffer(cl_position_s, 0, 0, num);
            cl_velocity_u.copyFromBuffer(cl_velocity_s, 0, 0, num);
            cl_veleval_u.copyFromBuffer(cl_veleval_s, 0, 0, num);
            cl_color_u.copyFromBuffer(cl_color_s, 0, 0, num);
    }

	//----------------------------------------------------------------------
    int SPH::setupTimers()
    {
        int time_offset = 5;
        timers["force_rigidbody"] = new EB::Timer("Force Rigid Body function", time_offset);
        timers["force_rigidbody_gpu"] = new EB::Timer("Force Rigid Body GPU kernel execution", time_offset);
        timers["density"] = new EB::Timer("Density function", time_offset);
        timers["density_gpu"] = new EB::Timer("Density GPU kernel execution", time_offset);
        timers["collision_wall"] = new EB::Timer("Collision wall function", time_offset);
        timers["cw_gpu"] = new EB::Timer("Collision Wall GPU kernel execution", time_offset);
        timers["collision_tri"] = new EB::Timer("Collision triangles function", time_offset);
        timers["ct_gpu"] = new EB::Timer("Collision Triangle GPU kernel execution", time_offset);
        timers["integrate"] = new EB::Timer("Integration function", time_offset);
        timers["leapfrog_gpu"] = new EB::Timer("LeapFrog Integration GPU kernel execution", time_offset);
        timers["euler_gpu"] = new EB::Timer("Euler Integration GPU kernel execution", time_offset);
		return 0;
    }

	//----------------------------------------------------------------------
    void SPH::prepareSorted()
    {

        vector<float4> f4vec(max_num);
        vector<float> fvec(max_num);
        std::fill(f4vec.begin(), f4vec.end(), float4(0.0f, 0.0f, 0.0f, 0.0f));
        std::fill(fvec.begin(), fvec.end(), 0.0f);

        //pure opencl buffers: these are deprecated
        cl_veleval_u = Buffer<float4>(ps->cli, f4vec);
        cl_veleval_s = Buffer<float4>(ps->cli, f4vec);
        cl_density_s = Buffer<float>(ps->cli, fvec);
        cl_xsph_s = Buffer<float4>(ps->cli, f4vec);

        //TODO make a helper constructor for buffer to make a cl_mem from a struct
        //Setup Grid Parameter structs
        std::vector<GridParams> gparams(0);
        gparams.push_back(grid_params);
        cl_GridParams = Buffer<GridParams>(ps->cli, gparams);

        //scaled Grid Parameters
        std::vector<GridParams> sgparams(0);
        sgparams.push_back(grid_params_scaled);
        cl_GridParamsScaled = Buffer<GridParams>(ps->cli, sgparams);
        // Size is the grid size + 1, the last index is used to signify how many particles are within bounds
        // That is a problem since the number of
        // occupied cells could be much less than the number of grid elements.
        printf("%d\n", grid_params.nb_cells);
        std::vector<unsigned int> gcells(grid_params.nb_cells+1);
        int minus = 0xffffffff;
        std::fill(gcells.begin(), gcells.end(), 666);

        cl_cell_indices_start = Buffer<unsigned int>(ps->cli, gcells);
        cl_cell_indices_end   = Buffer<unsigned int>(ps->cli, gcells);
        //printf("gp.nb_points= %d\n", gp.nb_points); exit(0);

     }

	//----------------------------------------------------------------------
    int SPH::addHose(int total_n, float4 center, float4 velocity, float radius, float4 color, float mass)
    {
        //in sph we just use sph spacing
        radius *= spacing;
        Hose *hose = new Hose(ps->settings->dt, total_n, center, velocity, radius, spacing, color);
        hoses.push_back(hose);
        //return the index
        return hoses.size()-1;
        //printf("size of hoses: %d\n", hoses.size());
    }
    void SPH::updateHose(int index, float4 center, float4 velocity, float radius, float4 color)
    {
        //we need to expose the vector of hoses somehow
        //doesn't seem right to make user manage an index
        //in sph we just use sph spacing
        radius *= spacing;
        hoses[index]->update(center, velocity, radius, spacing, color);
        //printf("size of hoses: %d\n", hoses.size());
    }
    void SPH::refillHose(int index, int refill)
    {
        hoses[index]->refill(refill);
    }

    void SPH::sprayHoses()
    {

        std::vector<float4> parts;
        for (int i = 0; i < hoses.size(); i++)
        {
            parts = hoses[i]->spray();
            if (parts.size() > 0)
                System::pushParticles(parts, hoses[i]->getVelocity(), hoses[i]->getColor());
        }
    }

	//----------------------------------------------------------------------
    void SPH::pushParticles(vector<float4> pos, vector<float4> vels, float4 color, float mass)
    {
        int nn = pos.size();
        if (num + nn > max_num)
        {
			printf("pushParticles: exceeded max nb(%d) of particles allowed\n", max_num);
            return;
        }
        std::vector<float4> cols(nn);
        //printf("color: %f %f %f %f\n", color.x, color.y, color.z, color.w);

        std::fill(cols.begin(), cols.end(),color);

#ifdef GPU
        glFinish();
        cl_position_u.acquire();
        cl_color_u.acquire();
        cl_velocity_u.acquire();
		// Allocate max_num particles on the GPU. That wastes memory, but is useful. 
		// There should be a way to update this during the simulation. 
        cl_position_u.copyToDevice(pos, num);
        cl_color_u.copyToDevice(cols, num);
        cl_velocity_u.copyToDevice(vels, num);
        settings->SetSetting("Number of Particles", num+nn);
        updateParams();
        cl_position_u.release();
        cl_color_u.release();
        cl_velocity_u.release();
#endif
        num += nn;  //keep track of number of particles we use
        renderer->setNum(num);
    }
	//----------------------------------------------------------------------
    void SPH::render()
    {
        renderer->render_box(grid->getBndMin(), grid->getBndMax());
        //renderer->render_table(grid->getBndMin(), grid->getBndMax());
        System::render();
    }
    void SPH::interact()
    {
            for(int j = 0;j<interactionSystem.size();j++)
            {
                //Naievely assume it is an rb system for now.
                //Need to come up with a good way to interact.
                timers["force_rigidbody"]->start();
                forceRB.execute(   num,
                    //cl_vars_sorted,
                    cl_position_s,
                    cl_velocity_s,
                    cl_force_s,
                    interactionSystem[j]->getPositionBuffer(),
                    interactionSystem[j]->getVelocityBuffer(),
                    interactionSystem[j]->getCellStartBuffer(),
                    interactionSystem[j]->getCellEndBuffer(),
                    cl_sphp,
                    //cl_GridParams,
                    cl_GridParamsScaled,
                    interactionSystem[j]->getSettings()->GetSettingAs<float>("Boundary Stiffness"),
                    interactionSystem[j]->getSettings()->GetSettingAs<float>("Boundary Dampening"),
                    clf_debug,
                    cli_debug);
                timers["force_rigidbody"]->stop();
            }
    }
}; //end namespace
