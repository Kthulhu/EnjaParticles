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
#include <limits.h>

#include "System.h"
//#include "../domain/UniformGrid.h"
#include "Domain.h"
#include "IV.h"

#include "common/Hose.h"

//for random
#include<time.h>

namespace rtps
{
	//----------------------------------------------------------------------
    System::System(RTPS *psfr, int n)
    {
        ps = psfr;
        max_num = n;
        num = 0;
        activeParticle = 0;

        settings = ps->settings;

		// I should be able to not specify this, but GPU restrictions ...

        resource_path = settings->GetSettingAs<string>("rtps_path");
        printf("resource path: %s\n", resource_path.c_str());

        //seed random
        srand ( time(NULL) );

        grid = settings->grid;

        setupTimers();
        //*** end Initialization
#ifdef CPU
        printf("RUNNING ON THE CPU\n");
#endif
#ifdef GPU
        printf("RUNNING ON THE GPU\n");
        prepareSorted();

        setRenderer();

        //should be more cross platform
        common_source_dir = resource_path + "/" + std::string(COMMON_CL_SOURCE_DIR);
        ps->cli->addIncludeDir(common_source_dir);
        printf("%s\n",common_source_dir.c_str());

        hash = Hash(common_source_dir, ps->cli, timers["hash_gpu"]);
        bitonic = Bitonic<unsigned int>(common_source_dir, ps->cli );
        //radix = Radix<unsigned int>(common_source_dir, ps->cli, max_num, 128);
        cellindices = CellIndices(common_source_dir, ps->cli, timers["ci_gpu"] );
        permute = Permute( common_source_dir, ps->cli, timers["perm_gpu"] );
        m2p = MeshToParticles(common_source_dir, ps->cli, timers["meshtoparticles_gpu"]);
#endif

    }

	//----------------------------------------------------------------------
    System::~System()
    {
        printf("System destructor\n");
        if (pos_vbo)//&& managed)
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
        if (velocity_vbo)// && managed)
        {
            glBindBuffer(1, velocity_vbo);
            glDeleteBuffers(1, (GLuint*)&velocity_vbo);
            velocity_vbo = 0;
        }
        if (force_vbo)// && managed)
        {
            glBindBuffer(1, force_vbo);
            glDeleteBuffers(1, (GLuint*)&force_vbo);
            force_vbo = 0;
        }
        if (active_cells_vbo)// && managed)
        {
            glBindBuffer(1, active_cells_vbo);
            glDeleteBuffers(1, (GLuint*)&active_cells_vbo);
            active_cells_vbo = 0;
        }
    }

	//----------------------------------------------------------------------
    /*void System::updateCPU()
    {
        cpuDensity();
        cpuPressure();
        cpuViscosity();
        cpuXSystem();
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
    void System::hash_and_sort()
    {
        //printf("hash\n");
        timers["hash"]->start();
        hash.execute(   num,
                //cl_vars_unsorted,
                cl_position_u,
                cl_sort_hashes,
                cl_sort_indices,
                //cl_sphp,
                cl_GridParams,
                clf_debug,
                cli_debug);
        timers["hash"]->stop();

        //printf("bitonic_sort\n");
        //defined in Sort.cpp
        timers["bitonic"]->start();
        bitonic_sort();
        timers["bitonic"]->stop();
        //timers["radix"]->start();
        //timers["bitonic"]->start();
        //radix_sort();
        //timers["bitonic"]->stop();
        //timers["radix"]->stop();
    }

	//----------------------------------------------------------------------
    int System::setupTimers()
    {
        int time_offset = 5;
        timers["update"] = new EB::Timer("Update loop", time_offset);
        timers["hash"] = new EB::Timer("Hash function", time_offset);
        timers["hash_gpu"] = new EB::Timer("Hash GPU kernel execution", time_offset);
        timers["cellindices"] = new EB::Timer("CellIndices function", time_offset);
        timers["ci_gpu"] = new EB::Timer("CellIndices GPU kernel execution", time_offset);
        timers["permute"] = new EB::Timer("Permute function", time_offset);
        timers["perm_gpu"] = new EB::Timer("Permute GPU kernel execution", time_offset);
        timers["force"] = new EB::Timer("Force function", time_offset);
        timers["force_gpu"] = new EB::Timer("Force GPU kernel execution", time_offset);
        timers["ds_gpu"] = new EB::Timer("DataStructures GPU kernel execution", time_offset);
        timers["bitonic"] = new EB::Timer("Bitonic Sort function", time_offset);
        timers["radix"] = new EB::Timer("Radix Sort function", time_offset);
		return 0;
    }

	//----------------------------------------------------------------------
    void System::printTimers()
    {
        printf("Number of Particles: %d\n", num);
        timers.printAll();
        std::ostringstream oss; 
        oss << "sph_timer_log_" << std::setw( 7 ) << std::setfill( '0' ) <<  num; 
        //printf("oss: %s\n", (oss.str()).c_str());
        timers.writeToFile(oss.str()); 
        renderer->printTimers();
    }

	//----------------------------------------------------------------------
    void System::prepareSorted()
    {
        vector<float4> f4vec(max_num);
        std::fill(f4vec.begin(), f4vec.end(), float4(0.0f, 0.0f, 0.0f, 0.0f));

        // VBO creation, TODO: should be abstracted to another class
        pos_vbo = createVBO(&f4vec[0], f4vec.size()*sizeof(float4), GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
        printf("pos vbo: %d\n", pos_vbo);
        col_vbo = createVBO(&f4vec[0], f4vec.size()*sizeof(float4), GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
        printf("color vbo: %d\n", col_vbo);
        velocity_vbo = createVBO(&f4vec[0], f4vec.size()*sizeof(float4), GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
        printf("velocity vbo: %d\n", velocity_vbo);
        force_vbo = createVBO(&f4vec[0], f4vec.size()*sizeof(float4), GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
        printf("force vbo: %d\n", force_vbo);
        active_cells_vbo = createVBO(&f4vec[0], f4vec.size()*sizeof(float4), GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
        printf("active_cells vbo: %d\n", active_cells_vbo);
        // end VBO creation

        //vbo buffers
        cl_position_u = Buffer<float4>(ps->cli, pos_vbo);
        cl_position_s = Buffer<float4>(ps->cli, f4vec);
        cl_color_u = Buffer<float4>(ps->cli, col_vbo);
        cl_color_s = Buffer<float4>(ps->cli, f4vec);
        cl_velocity_u = Buffer<float4>(ps->cli, velocity_vbo);
        cl_velocity_s = Buffer<float4>(ps->cli, f4vec);
        cl_force_s = Buffer<float4>(ps->cli, force_vbo);
        //cl_force_s = Buffer<float4>(ps->cli, f4vec);
        cl_active_cells = Buffer<float4>(ps->cli, active_cells_vbo);

        //setup debug arrays
        std::vector<int4> cliv(max_num);
        std::fill(cliv.begin(), cliv.end(),int4(0.0f, 0.0f, 0.0f, 0.0f));
        clf_debug = Buffer<float4>(ps->cli, f4vec);
        cli_debug = Buffer<int4>(ps->cli, cliv);
        

        std::vector<unsigned int> keys(max_num);
        //to get around limits of bitonic sort only handling powers of 2
        std::fill(keys.begin(), keys.end(), INT_MAX);
        cl_sort_indices  = Buffer<unsigned int>(ps->cli, keys);
        cl_sort_hashes   = Buffer<unsigned int>(ps->cli, keys);

        // For bitonic sort. Remove when bitonic sort no longer used
        // Currently, there is an error in the Radix Sort (just run both
        // sorts and compare outputs visually
        cl_sort_output_hashes = Buffer<unsigned int>(ps->cli, keys);
        cl_sort_output_indices = Buffer<unsigned int>(ps->cli, keys);

		printf("keys.size= %d\n", keys.size()); // 
     }

	//----------------------------------------------------------------------
    void System::setupDomain(float cell_size, float sim_scale)
    {
        grid->calculateCells(cell_size);

        grid_params.grid_min = grid->getMin();
        grid_params.grid_max = grid->getMax();
        grid_params.bnd_min  = grid->getBndMin();
        grid_params.bnd_max  = grid->getBndMax();

        //grid_params.bnd_min = float4(1, 1, 1,0);
        //grid_params.bnd_max =  float4(4, 4, 4, 0);

        grid_params.grid_res = grid->getRes();
        grid_params.grid_size = grid->getSize();
        grid_params.grid_delta = grid->getDelta();
        grid_params.nb_cells = (int) (grid_params.grid_res.x*grid_params.grid_res.y*grid_params.grid_res.z);

        /*
        grid_params.grid_inv_delta.x = 1. / grid_params.grid_delta.x;
        grid_params.grid_inv_delta.y = 1. / grid_params.grid_delta.y;
        grid_params.grid_inv_delta.z = 1. / grid_params.grid_delta.z;
        grid_params.grid_inv_delta.w = 1.;
        */

        grid_params_scaled.grid_min = grid_params.grid_min * sim_scale;
        grid_params_scaled.grid_max = grid_params.grid_max * sim_scale;
        grid_params_scaled.bnd_min  = grid_params.bnd_min * sim_scale;
        grid_params_scaled.bnd_max  = grid_params.bnd_max * sim_scale;
        grid_params_scaled.grid_res = grid_params.grid_res;
        grid_params_scaled.grid_size = grid_params.grid_size * sim_scale;
        grid_params_scaled.grid_delta = grid_params.grid_delta / sim_scale;
        grid_params_scaled.nb_cells = grid_params.nb_cells;

        grid_params.print();
        grid_params_scaled.print();
    }
    //----------------------------------------------------------------------
    int System::addBox(int nn, float4 min, float4 max, bool scaled, float4 color, float mass)
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
        pushParticles(rect, velo, color, mass);
        return rect.size();
    }

    void System::addBall(int nn, float4 center, float radius, bool scaled, float4 color, float mass)
    {
        float scale = 1.0f;
        vector<float4> sphere = addSphere(nn, center, radius, spacing, scale);
        float4 velo(0, 0, 0, 0);
        pushParticles(sphere,velo,color, mass);
    }

    void System::testDelete()
    {

        //cut = 1;
        std::vector<float4> poss(40);
        float4 posx(100.,100.,100.,1.);
        std::fill(poss.begin(), poss.end(),posx);
        //cl_vars_unsorted.copyToDevice(poss, max_num + 2);
        cl_position_u.acquire();
        cl_position_u.copyToDevice(poss);
        cl_position_u.release();
        ps->cli->queue.finish();
    }
	//----------------------------------------------------------------------
    void System::pushParticles(vector<float4> pos, float4 velo, float4 color, float mass)
    {
        int nn = pos.size();
        std::vector<float4> vels(nn);
        std::fill(vels.begin(), vels.end(), velo);
        pushParticles(pos, vels, color, mass);

    }

    void System::setRenderer()
    {
        switch(ps->settings->getRenderType())
        {
            case RTPSettings::SPRITE_RENDER:
                renderer = new SpriteRender(pos_vbo,col_vbo,num,ps->cli, ps->settings);
                //printf("spacing for radius %f\n", spacing);
                break;
            case RTPSettings::SCREEN_SPACE_RENDER:
                //renderer = new ScreenSpaceRender();
                renderer = new SSFRender(pos_vbo,col_vbo,num,ps->cli, ps->settings);
                break;
            case RTPSettings::RENDER:
                renderer = new Render(pos_vbo,col_vbo,num,ps->cli, ps->settings);
                break;
            case RTPSettings::SPHERE3D_RENDER:
                printf("new Sphere3DRender\n");
                renderer = new Sphere3DRender(pos_vbo,col_vbo,num,ps->cli, ps->settings);
                break;
            default:
                //should be an error
                renderer = new Render(pos_vbo,col_vbo,num,ps->cli, ps->settings);
                break;
        }
        //renderer->setParticleRadius(spacing*0.5);
		//renderer->setRTPS(
    }
	//----------------------------------------------------------------------
    void System::radix_sort()
    {
    try 
        {   
            int snum = nlpo2(num);
            if(snum < 1024)
            {   
                snum = 1024;
            }   
            //printf("sorting snum: %d", snum); 
            radix.sort(snum, &cl_sort_hashes, &cl_sort_indices);
        }   
        catch (cl::Error er) 
        {   
            printf("ERROR(radix sort): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
        }   

    }
	//----------------------------------------------------------------------
    void System::bitonic_sort()
    {
        try
        {
            int dir = 1;        // dir: direction
            //int batch = num;

            int arrayLength = nlpo2(num);
            //printf("num: %d\n", num);
            //printf("nlpo2(num): %d\n", arrayLength);
            //int arrayLength = max_num;
            //int batch = max_num / arrayLength;
            int batch = 1;

            //printf("about to try sorting\n");
            bitonic.Sort(batch, 
                        arrayLength, 
                        dir,
                        &cl_sort_output_hashes,
                        &cl_sort_output_indices,
                        &cl_sort_hashes,
                        &cl_sort_indices );

        }
        catch (cl::Error er)
        {
            printf("ERROR(bitonic sort): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
            exit(0);
        }

        ps->cli->queue.finish();

        /*
        int nbc = 10;
        std::vector<int> sh = cl_sort_hashes.copyToHost(nbc);
        std::vector<int> eci = cl_cell_indices_end.copyToHost(nbc);
    
        for(int i = 0; i < nbc; i++)
        {
            printf("before[%d] %d eci: %d\n; ", i, sh[i], eci[i]);
        }
        printf("\n");
        */


        cl_sort_hashes.copyFromBuffer(cl_sort_output_hashes, 0, 0, num);
        cl_sort_indices.copyFromBuffer(cl_sort_output_indices, 0, 0, num);

        /*
        scopy(num, cl_sort_output_hashes.getDevicePtr(), 
              cl_sort_hashes.getDevicePtr());
        scopy(num, cl_sort_output_indices.getDevicePtr(), 
              cl_sort_indices.getDevicePtr());
        */

        ps->cli->queue.finish();
#if 0
    
        printf("********* Bitonic Sort Diagnostics **************\n");
        int nbc = 20;
        //sh = cl_sort_hashes.copyToHost(nbc);
        //eci = cl_cell_indices_end.copyToHost(nbc);
        std::vector<unsigned int> sh = cl_sort_hashes.copyToHost(nbc);
        std::vector<unsigned int> si = cl_sort_indices.copyToHost(nbc);
        //std::vector<int> eci = cl_cell_indices_end.copyToHost(nbc);

    
        for(int i = 0; i < nbc; i++)
        {
            //printf("after[%d] %d eci: %d\n; ", i, sh[i], eci[i]);
            printf("sh[%d] %d si: %d\n ", i, sh[i], si[i]);
        }

#endif
    }
    void System::addParticleShape(GLuint tex3d,float scale,float4 min,float16 world,int resolution, float mass)
    {
        glFinish();
        cl::Image3DGL img(ps->cli->context,CL_MEM_READ_ONLY,GL_TEXTURE_3D,0,tex3d);
        std::vector<cl::Memory> objs;
        objs.push_back(img);
        ps->cli->queue.enqueueAcquireGLObjects(&objs,NULL,NULL);
        ps->cli->queue.finish();
        acquireGLBuffers();
        int tmpnum = m2p.execute(cl_position_u,cl_color_u,cl_velocity_u,num,img,scale,min,world,resolution,//debug
                clf_debug,
                cli_debug);
        printf("tmpnum = %d\n",tmpnum);
        num+=tmpnum;
        settings->SetSetting("Number of Particles", num);
        updateParams();
        renderer->setNum(num);
        //hash_and_sort();
        ps->cli->queue.enqueueReleaseGLObjects(&objs,NULL,NULL);
        ps->cli->queue.finish();
        releaseGLBuffers(); 
    }
    void System::acquireGLBuffers()
    {
        cl_position_u.acquire();
        cl_color_u.acquire();
        cl_velocity_u.acquire();
        cl_force_s.acquire();
        cl_active_cells.acquire();
    }
    void System::releaseGLBuffers()
    {
        cl_position_u.release();
        cl_color_u.release();
        cl_velocity_u.release();
        cl_force_s.release();
        cl_active_cells.release();
    }
    void System::render()
    {
        renderer->render();
        //renderer->renderVector(velocity_vbo);
        //renderer->renderVector(force_vbo,0.029706f);
    }
}; //end namespace
