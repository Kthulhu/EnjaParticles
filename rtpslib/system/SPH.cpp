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
        updateSPHP();

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
        
        ps->cli->addIncludeDir(sph_source_dir);

        //should be more cross platform
        sph_source_dir = resource_path + "/" + std::string(SPH_CL_SOURCE_DIR);

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
    }

	//----------------------------------------------------------------------
    SPH::~SPH()
    {
        printf("SPH destructor\n");
        if (pos_vbo)// && managed)
        {
            glBindBuffer(1, pos_vbo);
            glDeleteBuffers(1, (GLuint*)&pos_vbo);
            pos_vbo = 0;
        }
        if (col_vbo)// && managed)
        {
            glBindBuffer(1, col_vbo);
            glDeleteBuffers(1, (GLuint*)&col_vbo);
            col_vbo = 0;
        }

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
        glFinish();
        if (settings->has_changed()) updateSPHP();

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

        cl_position_u.acquire();
        cl_color_u.acquire();

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
                updateSPHP();
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

            integrate(); // includes boundary force
        }

        cl_position_u.release();
        cl_color_u.release();

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
        //int print_freq = 20000;
        int print_freq = 1000; //one second
        int time_offset = 5;
        timers["density"] = new EB::Timer("Density function", time_offset);
        timers["density_gpu"] = new EB::Timer("Density GPU kernel execution", time_offset);
        timers["force"] = new EB::Timer("Force function", time_offset);
        timers["force_gpu"] = new EB::Timer("Force GPU kernel execution", time_offset);
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
    void SPH::printTimers()
    {
        printf("Number of Particles: %d\n", num);
        timers.printAll();
        std::ostringstream oss; 
        oss << "sph_timer_log_" << std::setw( 7 ) << std::setfill( '0' ) <<  num; 
        //printf("oss: %s\n", (oss.str()).c_str());

        timers.writeToFile(oss.str()); 
    }

	//----------------------------------------------------------------------
    void SPH::prepareSorted()
    {
        System::prepareSorted();

        vector<float4> f4vec(max_num);
        vector<float> fvec(max_num);
        std::fill(f4vec.begin(), f4vec.end(), float4(0.0f, 0.0f, 0.0f, 0.0f));
        std::fill(fvec.begin(), fvec.end(), 0.0f);

        //pure opencl buffers: these are deprecated
        cl_veleval_u = Buffer<float4>(ps->cli, f4vec);
        cl_veleval_s = Buffer<float4>(ps->cli, f4vec);
        cl_density_s = Buffer<float>(ps->cli, fvec);
        cl_force_s = Buffer<float4>(ps->cli, f4vec);
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
     }



	//----------------------------------------------------------------------
    int SPH::addBox(int nn, float4 min, float4 max, bool scaled, float4 color)
    {
        float scale = 1.0f;
		#if 0
        if (scaled)
        {
            scale = sphp.simulation_scale;
        }
		#endif
		//printf("GEE inside addBox, before addRect, scale= %f\n", scale);
		//printf("GEE inside addBox, sphp.simulation_scale= %f\n", sphp.simulation_scale);
		printf("GEE addBox spacing = %f\n", spacing);
        vector<float4> rect = addRect(nn, min, max, spacing, scale);
        float4 velo(0, 0, 0, 0);
        pushParticles(rect, velo, color);
        return rect.size();
    }

    void SPH::addBall(int nn, float4 center, float radius, bool scaled)
    {
        float scale = 1.0f;
        if (scaled)
        {
            scale = sphp.simulation_scale;
        }
        vector<float4> sphere = addSphere(nn, center, radius, spacing, scale);
        float4 velo(0, 0, 0, 0);
        pushParticles(sphere,velo);
    }

	//----------------------------------------------------------------------
    int SPH::addHose(int total_n, float4 center, float4 velocity, float radius, float4 color)
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
                pushParticles(parts, hoses[i]->getVelocity(), hoses[i]->getColor());
        }
    }

	//----------------------------------------------------------------------
    void SPH::render()
    {
        renderer->render_box(grid->getBndMin(), grid->getBndMax());
        //renderer->render_table(grid->getBndMin(), grid->getBndMax());
        System::render();
    }
}; //end namespace
