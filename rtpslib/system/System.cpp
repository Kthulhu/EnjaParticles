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
#include "util.h"
#include "common/Hose.h"

//for random
#include<time.h>
using namespace std;

namespace rtps
{
	//----------------------------------------------------------------------
    System::System(RTPSSettings* set, CL* c)
    {
	dout<<"settings "<<set<<endl;

        settings = set;
        cli = c;
        acquiredGL=false;
        max_num = settings->GetSettingAs<unsigned int>("max_num_particles");
        num = settings->GetSettingAs<unsigned int>("num_particles");
        maxGravSources=settings->GetSettingAs<unsigned int>("max_gravity_sources");
        activeParticle = 0;
		// I should be able to not specify this, but GPU restrictions ...
        //seed random
        srand ( time(NULL) );

        grid = Domain(settings->GetSettingAs<float4>("domain_min"),settings->GetSettingAs<float4>("domain_max"));

        setupTimers();
        //*** end Initialization
#ifdef CPU
        dout<<"RUNNING ON THE CPU"<<endl;
#endif
#ifdef GPU
        dout<<"RUNNING ON THE GPU"<<endl;
        prepareSorted();

        //dout<<"Here"<<endl;
        //should be more cross platform
        string common_source_dir = settings->GetSettingAs<string>("rtps_path") + "/" + std::string(COMMON_CL_SOURCE_DIR);
        cli->addIncludeDir(common_source_dir);
        //dout<<common_source_dir.c_str()<<endl;

        hash = Hash(common_source_dir, cli, timers["hash_gpu"]);
        gravity = Gravity(common_source_dir, cli);
        bitonic = Bitonic<unsigned int>(common_source_dir, cli );
        //radix = Radix<unsigned int>(common_source_dir, cli, max_num, 128);
        cellindices = CellIndices(common_source_dir, cli, timers["ci_gpu"] );
        permute = Permute( common_source_dir, cli, timers["perm_gpu"] );
        m2p = MeshToParticles(common_source_dir, cli, timers["meshtoparticles_gpu"]);
        marchingcubes = MarchingCubes(common_source_dir, cli, timers["marchingcubes_gpu"],settings->GetSettingAs<unsigned int>("color_field_res","2"));
#endif

    }

	//----------------------------------------------------------------------
    System::~System()
    {
        //dout<<"System destructor"<<endl;
        if (pos_vbo)//&& managed)
        {
            glDeleteBuffers(1, (GLuint*)&pos_vbo);
            pos_vbo = 0;
        }
        if (col_vbo)// && managed)
        {
            glDeleteBuffers(1, (GLuint*)&col_vbo);
            col_vbo = 0;
        }
        if (velocity_vbo)// && managed)
        {
            glDeleteBuffers(1, (GLuint*)&velocity_vbo);
            velocity_vbo = 0;
        }
        if (force_vbo)// && managed)
        {
            glDeleteBuffers(1, (GLuint*)&force_vbo);
            force_vbo = 0;
        }
        if (active_cells_vbo)// && managed)
        {
            glDeleteBuffers(1, (GLuint*)&active_cells_vbo);
            active_cells_vbo = 0;
        }
	delete settings;
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
        timers["meshtoparticles_gpu"] = new EB::Timer("Mesh to particles GPU kernel", time_offset);
        timers["marchingcubes"] = new EB::Timer("MarchingCubes function", time_offset);
        timers["marchingcubes_gpu"] = new EB::Timer("MarchingCubes GPU kernel execution", time_offset);
		return 0;
    }

	//----------------------------------------------------------------------


	//----------------------------------------------------------------------
    void System::prepareSorted()
    {
        vector<float4> f4vec(max_num);
        std::fill(f4vec.begin(), f4vec.end(), float4(0.0f, 0.0f, 0.0f, 0.0f));

        //size should reflect the maximum number of neighbors.
        vector<float4> f4activevec(100*2);
        std::fill(f4activevec.begin(), f4activevec.end(), float4(0.0f, 0.0f, 0.0f, 0.0f));

        vector<float> fvec(max_num);
        std::fill(fvec.begin(), fvec.end(), 0.0f);

        vector<unsigned int> uivec(max_num);
        std::fill(uivec.begin(), uivec.end(), 0);
        //dout<<"Here"<<endl;
        // VBO creation, TODO: should be abstracted to another class
        pos_vbo = createVBO(&f4vec[0], f4vec.size()*sizeof(float4), GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
        //dout<<"Here"<<endl;
        //dout<<"pos vbo: "<< pos_vbo<<endl;
        col_vbo = createVBO(&f4vec[0], f4vec.size()*sizeof(float4), GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
        //dout<<"color vbo: "<< col_vbo<<endl;
        velocity_vbo = createVBO(&f4vec[0], f4vec.size()*sizeof(float4), GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
        //dout<<"velocity vbo: "<< velocity_vbo<<endl;
        force_vbo = createVBO(&f4vec[0], f4vec.size()*sizeof(float4), GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
        //dout<<"force vbo: "<< force_vbo<<endl;
        active_cells_vbo = createVBO(&f4activevec[0], f4activevec.size()*sizeof(float4), GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
        //active_lines_vbo = createVBO(&f4activevec[0], f4activevec.size()*sizeof(float4), GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
        //active_col_vbo = createVBO(&f4activevec[0], f4activevec.size()*sizeof(float4), GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
        //dout<<"active cells vbo: "<< active_cells_vbo<<endl;
        //dout<<"address of cli = "<<cli<<endl;
        // end VBO creation

        try
        {
        //vbo buffers
        cl_position_u = Buffer<float4>(cli, pos_vbo);
        cl_position_s = Buffer<float4>(cli, f4vec);
        cl_color_u = Buffer<float4>(cli, col_vbo);
        cl_color_s = Buffer<float4>(cli, f4vec);
        cl_velocity_u = Buffer<float4>(cli, velocity_vbo);
        cl_velocity_s = Buffer<float4>(cli, f4vec);
        cl_force_s = Buffer<float4>(cli, force_vbo);
        //cl_force_s = Buffer<float4>(cli, f4vec);
        cl_active_cells = Buffer<float4>(cli, active_cells_vbo);
        //cl_active_lines = Buffer<float4>(cli, active_lines_vbo);
        //cl_active_col = Buffer<float4>(cli, active_col_vbo);
        cl_mass_u = Buffer<float>(cli, fvec);
        cl_mass_s = Buffer<float>(cli, fvec);
        cl_objectIndex_u = Buffer<unsigned int>(cli, uivec);
        cl_objectIndex_s = Buffer<unsigned int>(cli, uivec);
        //dout<<"Here"<<endl;

        //setup debug arrays
        std::vector<int4> cliv(max_num);
        std::fill(cliv.begin(), cliv.end(),int4(0.0f, 0.0f, 0.0f, 0.0f));
        clf_debug = Buffer<float4>(cli, f4vec);
        cli_debug = Buffer<int4>(cli, cliv);

        //dout<<"grav_sources = "<<settings->GetSettingAs<string >("gravity_sources")<<endl;
        //dout<<"grav_mass = "<<settings->GetSettingAs<string >("gravity_mass")<<endl;
        //dout<<"grav_alphas = "<<settings->GetSettingAs<string >("gravity_alphas")<<endl;

        //Gravity
        vector<float4> grav_sources = settings->GetSettingAs<vector<float4> >("gravity_sources");
        cl_pointSources = Buffer<float4>(cli, grav_sources);
        vector<float> grav_masses = settings->GetSettingAs<vector<float> >("gravity_mass");
        cl_massSources = Buffer<float>(cli, grav_masses);
        vector<float> grav_alphas = settings->GetSettingAs<vector<float> >("gravity_alphas");
        cl_alphaSources = Buffer<float>(cli, grav_alphas);
        for(int i=0;i<grav_alphas.size();i++)
        {
            dout<<"Alphas # "<<i<<": "<<grav_alphas[i]<<std::endl;
        }
        numGravSources=grav_sources.size();
        maxGravSources=grav_sources.size();

        std::vector<unsigned int> keys(max_num);
        //to get around limits of bitonic sort only handling powers of 2
        std::fill(keys.begin(), keys.end(), INT_MAX);
        cl_sort_indices  = Buffer<unsigned int>(cli, keys);
        cl_sort_hashes   = Buffer<unsigned int>(cli, keys);

        // For bitonic sort. Remove when bitonic sort no longer used
        // Currently, there is an error in the Radix Sort (just run both
        // sorts and compare outputs visually
        cl_sort_output_hashes = Buffer<unsigned int>(cli, keys);
        cl_sort_output_indices = Buffer<unsigned int>(cli, keys);
		dout<<"keys.size= "<< keys.size()<<endl; //
        }
        catch (cl::Error er)
        {
            cerr<<"ERROR: "<<er.what()<<"("<< CL::oclErrorString(er.err())<<")"<<endl;
        }
     }

	//----------------------------------------------------------------------
    void System::setupDomain(float cell_size, float sim_scale)
    {
        grid.calculateCells(cell_size);

        grid_params.grid_min = grid.getMin();
        grid_params.grid_max = grid.getMax();
        grid_params.bnd_min  = grid.getBndMin();
        grid_params.bnd_max  = grid.getBndMax();

        //grid_params.bnd_min = float4(1, 1, 1,0);
        //grid_params.bnd_max =  float4(4, 4, 4, 0);

        grid_params.grid_res = grid.getRes();
        grid_params.grid_size = grid.getSize();
        grid_params.grid_delta = grid.getDelta();
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

    void System::addTorus(int nn, float4 center, float innerRadius, float outerRadius, float thickness, float4 color, float mass, float innerVel, float outerVel)
    {
        float scale = 1.0f;
        vector<float4> initVel;
        vector<float4> torus = generateTorus(nn, center,innerRadius, outerRadius, thickness,spacing, scale, innerVel, outerVel,initVel);
        pushParticles(torus,initVel,color, mass);
    }

    void System::addPointSource(float4& pointSource, float massSource)
    {
        if(numGravSources<maxGravSources)
        {
           cl_pointSources.copyToDevice(pointSource,numGravSources);
           cl_massSources.copyToDevice(massSource,numGravSources);
           numGravSources++;
        }
    }

    void System::testDelete()
    {
        std::vector<float4> poss(40);
        float4 posx(100.,100.,100.,1.);
        std::fill(poss.begin(), poss.end(),posx);
        //cl_vars_unsorted.copyToDevice(poss, max_num + 2);
        cl_position_u.acquire();
        cl_position_u.copyToDevice(poss);
        cl_position_u.release();
        cli->queue.finish();
    }
	//----------------------------------------------------------------------
    void System::pushParticles(vector<float4> pos, float4 velo, float4 color, float mass)
    {
        int nn = pos.size();
        std::vector<float4> vels(nn);
        std::fill(vels.begin(), vels.end(), velo);
        pushParticles(pos, vels, color, mass);

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
            cout<<"ERROR(radix sort): "<< er.what()<<"("<< CL::oclErrorString(er.err())<<")"<<endl;
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
            cout<<"ERROR(bitonic sort): "<< er.what()<<"("<< CL::oclErrorString(er.err())<<")"<<endl;
        }

        cli->queue.finish();

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

        cli->queue.finish();
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
    void System::addParticleShape(GLuint tex3d,float min,float max,float16 world,int voxelResolution,float4 velo, float4 color,float mass)
    {
//        glFinish();
        //TODO: move the particle vector creation to the particle shape class.
        //This will significantly improve performance of adding a shape to the system.
        //Currently we must loop through the entire 3D volume texture upon each insert!
        vector<float4> vec;
        glBindTexture(GL_TEXTURE_3D_EXT,tex3d);
        GLubyte* image = new GLubyte[voxelResolution*voxelResolution*voxelResolution*4];
        glGetTexImage(GL_TEXTURE_3D_EXT,0,GL_RGBA,GL_UNSIGNED_BYTE,image);
        float scale = max-min;
        for(int k = 0; k<voxelResolution; k++)
        {
            for(int j=0; j<voxelResolution; j++)
            {
                for(int i=0;i<voxelResolution;i++)
                {
                    //Check the red channel. If it is non zero then we need a particle here.
                    if(image[(i*4)+(j*voxelResolution*4)+(k*voxelResolution*voxelResolution*4)]>0)
                    {
                        float4 pos;
                        pos.x = ((j/(float)(voxelResolution-1))-0.5f)*scale;
                        pos.y = ((i/(float)(voxelResolution-1))-0.5f)*scale;
                        pos.z = ((k/(float)(voxelResolution-1))-0.5f)*scale;
                        pos.w = 1.0f;
                        pos = world*pos;
                        vec.push_back(pos);
                    }
                }
            }
        }
        glBindTexture(GL_TEXTURE_3D_EXT,0);
        delete[] image;

        dout<<"Num particles from voxel = "<<vec.size()<<endl;
        if(vec.size()==0)
            cerr<<"3D texture is empty!!! Can't create particles!"<<endl;
        else
            pushParticles(vec,velo,color,mass);
    }
    void System::acquireGLBuffers()
    {
        cl_position_u.acquire();
        cl_color_u.acquire();
        cl_velocity_u.acquire();
        cl_force_s.acquire();
        //cl_active_cells.acquire();
        acquiredGL=true;
    }
    void System::releaseGLBuffers()
    {
        cl_position_u.release();
        cl_color_u.release();
        cl_velocity_u.release();
        cl_force_s.release();
        //cl_active_cells.release();
        acquiredGL=false;
    }
    RTPSSettings* System::getSettings() {return settings;}

	void System::addInteractionSystem(System* interact)
	{
		interactionSystem.push_back(interact);
	}
}; //end namespace
