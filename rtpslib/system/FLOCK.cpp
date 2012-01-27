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

#include "System.h"
#include "FLOCK.h"
#include "Domain.h"
#include "IV.h"

#include "common/Hose.h"

//for random
#include<time.h>

namespace rtps
{

//----------------------------------------------------------------------
FLOCK::FLOCK(RTPS *psfr, int n):System(psfr,n)
{
    //seed random
    srand ( time(NULL) );

    std::vector<FLOCKParameters> vparams(0);
    vparams.push_back(flock_params);
    cl_FLOCKParameters= Buffer<FLOCKParameters>(cli, vparams);

    calculate();
    updateParams();

    spacing = settings->GetSettingAs<float>("Spacing");

    //set up the grid
    setupDomain(flock_params.smoothing_distance/flock_params.simulation_scale,flock_params.simulation_scale);
    
    //set up the timers 
    setupTimers();

    //setup the sorted and unsorted arrays
    prepareSorted();

#ifdef CPU
    dout<<"RUNNING ON THE CPU"<<endl;
#endif
#ifdef GPU
    dout<<"RUNNING ON THE GPU"<<endl;

    //should be more cross platform
    string flock_source_dir = settings->GetSettingAs<string>("rtps_path") + "/" + std::string(FLOCK_CL_SOURCE_DIR);

    cli->addIncludeDir(flock_source_dir);
    
    rules = Rules(flock_source_dir, cli, timers["rules_gpu"]);
    euler_integration = EulerIntegration(flock_source_dir, cli, timers["euler_gpu"]);
#endif
        //renderer->setParticleRadius(spacing);
}

//----------------------------------------------------------------------
FLOCK::~FLOCK()
{
    printf("FLOCK destructor\n");
    Hose* hose;
    int hs = hoses.size();  
    for(int i = 0; i < hs; i++)
    {
        hose = hoses[i];
        delete hose;
    }

}

//----------------------------------------------------------------------
void FLOCK::update()
{
#ifdef CPU
    updateCPU();
#endif
#ifdef GPU
    updateGPU();
#endif
}

//----------------------------------------------------------------------
/*void FLOCK::updateCPU()
{
    if(settings->has_changed())
        updateParams();
    
    cpuRules();
    cpuEulerIntegration();

    // mymese debugging
#if 0
    for(int i = 0; i < num; i+=64)
    {
        printf("particle %d, positions: %f %f %f  \n", positions[i].x, positions[i].y, positions[i].z);
    }
#endif

    glBindBuffer(GL_ARRAY_BUFFER, pos_vbo);
    glBufferData(GL_ARRAY_BUFFER, num * sizeof(float4), &positions[0], GL_DYNAMIC_DRAW);
    
}*/

//----------------------------------------------------------------------
void FLOCK::updateGPU()
{
    //mymese debbug
    dout<<"smoth_dist: "<< flock_params.smoothing_distance<<endl;
    dout<<"radius: "<< flock_params.search_radius<<endl;
    dout<<"min dist: "<< flock_params.min_dist<<endl;

    timers["update"]->start();

    if(settings->hasChanged())
        updateParams();

    //sub-intervals
    int sub_intervals = settings->GetSettingAs<float>("sub_intervals");  //should be a setting
    
    for (int i=0; i < sub_intervals; i++)
    {
        sprayHoses();
    }

    
    for(int i=0; i < sub_intervals; i++)
    {
        hash_and_sort();

        timers["cellindices"]->start();
        int nc = cellindices.execute(   num,
            cl_sort_hashes,
            cl_sort_indices,
            cl_cell_indices_start,
            cl_cell_indices_end,
            cl_GridParams,
            grid_params.nb_cells,
            clf_debug,
            cli_debug);
        timers["cellindices"]->stop();
       
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
            cl_mass_u,
            cl_mass_s,
            cl_objectIndex_u,
            cl_objectIndex_s,
            cl_sort_indices,
            cl_GridParams,
            clf_debug,
            cli_debug);
        timers["permute"]->stop();

        if (nc <= num && nc >= 0)
        {
            dout<<"SOME PARTICLES WERE DELETED!"<<endl;
            dout<<"nc: "<<nc<<" num: "<<num<<endl;

            deleted_pos.resize(num-nc);
            deleted_vel.resize(num-nc);
            
            cl_position_s.copyToHost(deleted_pos, nc); 
            cl_velocity_s.copyToHost(deleted_vel, nc);
 
            num = nc;
            settings->SetSetting("num_particles", num);
            
            updateParams();
            //renderer->setNum(flock_params.num);
            
            //need to copy sorted arrays into unsorted arrays
            call_prep(2);
            
            hash_and_sort();
        }

        timers["rules"]->start();

        if(flock_params.w_sep > 0.f || flock_params.w_align > 0.f || flock_params.w_coh > 0.f || flock_params.w_goal > 0.f || flock_params.w_avoid > 0.f){
            rules.execute(   num,
                settings->target,
                cl_position_s,
                cl_velocity_s,
                cl_flockmates_s,
                cl_separation_s,
                cl_alignment_s,
                cl_cohesion_s,
                cl_goal_s,
                cl_avoid_s,
                cl_cell_indices_start,
                cl_cell_indices_end,
                cl_GridParamsScaled,
                cl_FLOCKParameters,
                clf_debug,
                cli_debug);
        }
        
        timers["rules"]->stop();
        

    }


    timers["update"]->stop();
}

//----------------------------------------------------------------------
void FLOCK::integrate()
{
    timers["integrate"]->start();
    euler_integration.execute(num,
        settings->dt,
        settings->two_dimensional,
        cl_position_u,
        cl_position_s,
        cl_velocity_u,
        cl_velocity_s,
        cl_separation_s,
        cl_alignment_s,
        cl_cohesion_s,
        cl_goal_s,
        cl_avoid_s,
        cl_leaderfollowing_s,
        cl_sort_indices,
        cl_FLOCKParameters,
        cl_GridParamsScaled,
        //debug
        clf_debug,
        cli_debug);

    // mymese debugging
#if 0  
    if(num > 0)
    {
        vector<int4> cli(num);
        cli_debug.copyToHost(cli);

        vector<float4> clf(num);
        clf_debug.copyToHost(clf);

        for(int i = 0; i < 4; i++)
        {
            printf("w1 = %d\t w2 = %d\t w3 = %d\t w4 = %d\n", cli[i].x, cli[i].y, cli[i].z, cli[i].w);
            printf("clf[%d] = %f %f %f %f\n", i, clf[i].x, clf[i].y, clf[i].z, clf[i].w);
        }
		printf("num= %d\n", num);
        printf("\n\n");
    }
#endif

        timers["integrate"]->stop();
}

//----------------------------------------------------------------------
void FLOCK::call_prep(int stage)
{
    //Replace with enqueueCopyBuffer

    cl_position_u.copyFromBuffer(cl_position_s, 0, 0, num);
    cl_velocity_u.copyFromBuffer(cl_velocity_s, 0, 0, num);
    cl_veleval_u.copyFromBuffer(cl_veleval_s, 0, 0, num);
    cl_color_u.copyFromBuffer(cl_color_s, 0, 0, num);
}

//----------------------------------------------------------------------
int FLOCK::setupTimers()
{
    int time_offset = 5;
    timers["integrate"] = new EB::Timer("Integration kernel execution", time_offset);
    timers["euler_gpu"] = new EB::Timer("Euler integration", time_offset);
    timers["rules"] = new EB::Timer("Computes all the rules", time_offset);
    timers["rules_gpu"] = new EB::Timer("Computes all the rules in the GPU", time_offset);

	return 0;
}

//----------------------------------------------------------------------
void FLOCK::prepareSorted()
{
 
    vector<int4> i4Vec(max_num);
    vector<float4> f4Vec(max_num);
    fill(i4Vec.begin(), i4Vec.end(), int4(0,0,0,0));
    fill(f4Vec.begin(), f4Vec.end(), float4(0.0f, 0.0f, 0.0f, 0.0f));
    cl_flockmates_s= Buffer<int4>(cli, flockmates);
    cl_separation_s = Buffer<float4>(cli, separation);
    cl_alignment_s = Buffer<float4>(cli, alignment);
    cl_cohesion_s = Buffer<float4>(cli, cohesion);
    cl_goal_s = Buffer<float4>(cli, goal);
    cl_avoid_s = Buffer<float4>(cli, avoid);
    cl_leaderfollowing_s = Buffer<float4>(cli, leaderfollowing);
            //TODO make a helper constructor for buffer to make a cl_mem from a struct
        //Setup Grid Parameter structs
        vector<GridParams> gparams(0);
        gparams.push_back(grid_params);
        cl_GridParams = Buffer<GridParams>(cli, gparams);

        //scaled Grid Parameters
        vector<GridParams> sgparams(0);
        sgparams.push_back(grid_params_scaled);
        cl_GridParamsScaled = Buffer<GridParams>(cli, sgparams);
                // Size is the grid size + 1, the last index is used to signify how many particles are within bounds
        // That is a problem since the number of
        // occupied cells could be much less than the number of grid elements.
        dout<<"Number of Grid Cells: "<< grid_params.nb_cells<<endl;
        vector<unsigned int> gcells(grid_params.nb_cells+1);
        int minus = 0xffffffff;
        std::fill(gcells.begin(), gcells.end(), 666);

        cl_cell_indices_start = Buffer<unsigned int>(cli, gcells);
        cl_cell_indices_end   = Buffer<unsigned int>(cli, gcells);

}

//----------------------------------------------------------------------
int FLOCK::addHose(int total_n, float4 center, float4 velocity, float radius, float4 color)
{
    radius *= spacing;
    Hose* hose = new Hose(settings->dt, total_n, center, velocity, radius, spacing, color);
    hoses.push_back(hose);
    return hoses.size()-1;

}

//----------------------------------------------------------------------
void FLOCK::updateHose(int index, float4 center, float4 velocity, float radius, float4 color)
{
    //we need to expose the vector of hoses somehow doesn't seem right to make user manage an index
    radius *= spacing;
    hoses[index]->update(center, velocity, radius, spacing, color);
}

//----------------------------------------------------------------------
void FLOCK::sprayHoses()
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
void FLOCK::pushParticles(vector<float4> pos, vector<float4> vels, float4 color)
{
    int nn = pos.size();
    
    // if we have reach max num of particles, then return
    if (num + nn > max_num) {return;}
    
    vector<float4> cols(nn);
    fill(cols.begin(), cols.end(),color); //BLENDER

#ifdef CPU
    std::copy(pos.begin(), pos.end(), positions.begin()+num);
#endif

#ifdef GPU
    glFinish();
    cl_position_u.acquire();
    cl_color_u.acquire();
    cl_velocity_u.acquire();
 
    cl_position_u.copyToDevice(pos, num);
    cl_color_u.copyToDevice(cols, num);
    cl_velocity_u.copyToDevice(vels, num);

    settings->SetSetting("num_particles", num+nn);
    updateParams();

    cl_velocity_u.release();
    cl_color_u.release();
    cl_position_u.release();

#else
    num += nn;  //keep track of number of particles we use
}

//----------------------------------------------------------------------
/*void FLOCK::render()
{
    renderer->render_box(grid->getBndMin(), grid->getBndMax());
    System::render();
}*/
/*void FLOCK::interact()
{
    
}*/
}
