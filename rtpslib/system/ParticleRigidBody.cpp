/****************************************************************************************
* Real-Time Particle System - An OpenCL based Particle system developed to run on modern GPUs. Includes ParticleRigidBody fluid simulations.
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

#include "ParticleRigidBody.h"
#include "Domain.h"
#include "IV.h"

//for random
#include<time.h>
#define sq(x)(x*x)

namespace rtps
{

	//----------------------------------------------------------------------
    ParticleRigidBody::ParticleRigidBody(RTPSSettings* set, CL* c) :System(set,c), curRigidbodyID(0)
    {

        std::vector<ParticleRigidBodyParams> vparams(0);
        vparams.push_back(prbp);
        cl_prbp = Buffer<ParticleRigidBodyParams>(cli, vparams);

        calculate();
        updateParams();

        //settings->printSettings();

        spacing = settings->GetSettingAs<float>("spacing");

        static_num=0;
        //ParticleRigidBody settings depend on number of particles used
        //calculateParticleRigidBodySettings();
        //set up the grid
        setupDomain(prbp.smoothing_distance/prbp.simulation_scale,prbp.simulation_scale);

        setupTimers();

#ifdef GPU
        dout<<"RUNNING ON THE GPU"<<endl;

        //setup the sorted and unsorted arrays
        prepareSorted();

        //should be more cross platform
        string rigidbody_source_dir = settings->GetSettingAs<string>("rtps_path") + "/" + string(PARTICLE_RIGIDBODY_CL_SOURCE_DIR);
        cli->addIncludeDir(rigidbody_source_dir);
        force = PRBForce(rigidbody_source_dir, cli, timers["force_gpu"]);
        forceFluid = PRBForceFluid(rigidbody_source_dir, cli, timers["force_fluid_gpu"]);
        forceStatic = PRBForceStatic(rigidbody_source_dir, cli, timers["force_static_gpu"]);
        sscan = PRBSegmentedScan(rigidbody_source_dir, cli, timers["segmented_scan_gpu"]);
        updateParticles = PRBUpdateParticles(rigidbody_source_dir, cli, timers["update_particles_gpu"]);


        //could generalize this to other integration methods later (leap frog, RK4)
        if (settings->GetSettingAs<string>("integrator")=="leapfrog")
        {
            //loadLeapFrog();
            leapfrog = PRBLeapFrog(rigidbody_source_dir, cli, timers["leapfrog_gpu"]);
        }
        else if (settings->GetSettingAs<string>("integrator")=="euler")
        {
            //loadEuler();
            euler = PRBEuler(rigidbody_source_dir, cli, timers["euler_gpu"]);
        }

#endif

        //renderer->setParticleRadius(spacing);
    }

	//----------------------------------------------------------------------
    ParticleRigidBody::~ParticleRigidBody()
    {
        dout<<"Particle Rigid Body Destructor"<<endl;
    }

	//----------------------------------------------------------------------
    void ParticleRigidBody::update()
    {
#ifdef GPU
        updateGPU();
#endif
    }

	//----------------------------------------------------------------------
    void ParticleRigidBody::updateGPU()
    {
        timers["update"]->start();
        if (settings->hasChanged()) updateParams();

        //settings->printSettings();

        //int sub_intervals = 3;  //should be a setting
        int sub_intervals =  settings->GetSettingAs<float>("sub_intervals");
        //this should go in the loop but avoiding acquiring and releasing each sub
        //interval for all the other calls.
        //this does end up acquire/release everytime sprayHoses calls pushparticles
        //should just do try/except?

        for (int i=0; i < sub_intervals; i++)
        {
            hash_and_sort();

			//------------------
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

			//-----------------
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
                /*cl_spring_coef_u,
                cl_spring_coef_s,
                cl_dampening_coef_u,
                cl_dampening_coef_s,*/
                cl_sort_indices,
                //cl_prbp,
                cl_GridParams,
                clf_debug,
                cli_debug);
            timers["permute"]->stop();
			//printf("exit after fluid permute\n");
			//if (num > 0) exit(0);

			//---------------------
            /*if (nc <= num && nc >= 0)
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
                //prbp.num = num;
                updateParams();
                //renderer->setNum(prbp.num);

                //need to copy sorted arrays into unsorted arrays
//**** PREP(2)
                call_prep(2);
                //printf("HOW MANY NOW? %d\n", num);
                hash_and_sort();
                                //we've changed num and copied sorted to unsorted. skip this iteration and do next one
                //this doesn't work because sorted force etc. are having an effect?
                //continue;
            }*/


			//-------------------------------------
            //if(num >0) printf("force\n");
            timers["force"]->start();
            force.execute(   num,
                //cl_vars_sorted,
                cl_position_s,
                cl_velocity_s,
                //cl_veleval_s,
                cl_force_s,
                cl_mass_s,
                cl_objectIndex_s,
                //cl_spring_coef_s,
                //cl_dampening_coef_s,
                cl_sort_indices,
                cl_cell_indices_start,
                cl_cell_indices_end,
                cl_prbp,
                //cl_GridParams,
                cl_GridParamsScaled,
                clf_debug,
                cli_debug);
            timers["force"]->stop();
            timers["force_static"]->start();
            forceStatic.execute( num,
                cl_position_s,
                cl_velocity_s,
                cl_force_s,
                cl_mass_s,
                cl_static_position_s,
                cl_sort_indices,
                cl_cell_static_indices_start,
                cl_cell_static_indices_end,
                cl_prbp,
                cl_GridParamsScaled,
                clf_debug,
                cli_debug);
            timers["force_static"]->stop();
            gravity.execute(num,
                    numGravSources,
                    cl_pointSources,
                    cl_massSources,
                    cl_alphaSources,
                    cl_position_s,
                    cl_mass_s,
                    cl_force_s,
                    prbp.simulation_scale);
        }


        timers["update"]->stop();
    }

    void ParticleRigidBody::integrate()
    {
        timers["integrate"]->start();
        if (settings->GetSettingAs<string>("integrator")=="euler")
        {
            //euler();
            euler.execute(num,
                settings->GetSettingAs<float>("time_step"),
                cl_comLinearForce,
                cl_comTorqueForce,
                cl_comVelEval,
                cl_comAngVelEval,
                cl_comPos,
                cl_comRot,
                cl_invInertialTensor,
                cl_rbMass,
                prbp.gravity,
                rbParticleIndex.size(),
                //debug
                cl_prbp,
                clf_debug,
                cli_debug);


        }
        else if (settings->GetSettingAs<string>("integrator")=="leapfrog")
        {
            for(int i = 0;i<rbParticleIndex.size(); i++)
            {
                dout<<i<<" of "<<rbParticleIndex.size()<<": start = "<<rbParticleIndex[i].x<<" end = "<<rbParticleIndex[i].y<<endl;
            }
            //leapfrog();
            leapfrog.execute(num,
                settings->GetSettingAs<float>("time_step"),
                cl_comLinearForce,
                cl_comTorqueForce,
                cl_comVel,
                cl_comAngVel,
                cl_comVelEval,
                cl_comAngVelEval,
                cl_comPos,
                cl_comRot,
                cl_invInertialTensor,
                cl_rbMass,
                rbParticleIndex.size(),
                //debug
                cl_prbp,
                clf_debug,
                cli_debug);

        }

		static int count=0;

    	timers["integrate"]->stop();
    }

	// GE: WHY IS THIS NEEDED?
	//----------------------------------------------------------------------
    void ParticleRigidBody::call_prep(int stage)
    {
		// copy from sorted to unsorted arrays at the beginning of each
		// iteration
		// copy from cl_position_s to cl_position_u
		// Only called if number of fluid particles changes from one iteration
		// to the other

            cl_position_u.copyFromBuffer(cl_position_s, 0, 0, num);
            cl_velocity_u.copyFromBuffer(cl_velocity_s, 0, 0, num);
            cl_veleval_u.copyFromBuffer(cl_veleval_s, 0, 0, num);
    }

	//----------------------------------------------------------------------
    int ParticleRigidBody::setupTimers()
    {
        int time_offset = 5;
        timers["force_fluid"] = new EB::Timer("Force Fluid function", time_offset);
        timers["force_fluid_gpu"] = new EB::Timer("Force Fluid GPU kernel execution", time_offset);
        timers["force_static"] = new EB::Timer("Force Static function", time_offset);
        timers["force_static_gpu"] = new EB::Timer("Force Static GPU kernel execution", time_offset);
        timers["integrate"] = new EB::Timer("Integration function", time_offset);
        timers["leapfrog_gpu"] = new EB::Timer("LeapFrog Integration GPU kernel execution", time_offset);
        timers["euler_gpu"] = new EB::Timer("Euler Integration GPU kernel execution", time_offset);
        timers["segmented_scan"] = new EB::Timer("Segmented scan function", time_offset);
        timers["segmented_scan_gpu"] = new EB::Timer("Segmented scan GPU kernel execution", time_offset);
        timers["update_particles"] = new EB::Timer("Update Particles function", time_offset);
        timers["update_particles_gpu"] = new EB::Timer("Update Particles GPU kernel execution", time_offset);
        return 0;
    }

	//----------------------------------------------------------------------
    void ParticleRigidBody::prepareSorted()
    {
        vector<float4> f4Vec(max_num);
        //FIXME: We are assuming that the maximum number of rigid bodies are max_num/4
        //That means that we assume that each rigidbody is represented by no less than 4 particles
        rbParticleIndex.resize(max_num/4);
        vector<float4> rbf4Vec(max_num/4);
        vector<float16> rbf16Vec(max_num/4);
        vector<float4> rotf4Vec(max_num/4);
        vector<float> rbfVec(max_num/4);

        fill(f4Vec.begin(), f4Vec.end(), float4(0.0f, 0.0f, 0.0f, 0.0f));
        fill(rbf4Vec.begin(), rbf4Vec.end(), float4(0.0f, 0.0f, 0.0f, 0.0f));
        fill(rbf16Vec.begin(), rbf16Vec.end(), float16(0.0f,0.0f,0.0f,0.0f,
                                                            0.0f,0.0f,0.0f,0.0f,
                                                            0.0f,0.0f,0.0f,0.0f,
                                                            0.0f,0.0f,0.0f,0.0f));
        fill(rotf4Vec.begin(), rotf4Vec.end(), float4(0.0f, 0.0f, 0.0f, 1.0f));
        fill(rbfVec.begin(), rbfVec.end(),0.0f);
        fill(rbParticleIndex.begin(),rbParticleIndex.end(),int2(0,0));
        staticVBO = createVBO(&f4Vec[0], f4Vec.size()*sizeof(float4), GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
        comPosVBO = createVBO(&rbf4Vec[0], rbf4Vec.size()*sizeof(float4), GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
        comRotationVBO = createVBO(&rotf4Vec[0], rotf4Vec.size()*sizeof(float4), GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
        cl_static_position_u=Buffer<float4>(cli,staticVBO);
        cl_static_position_s=Buffer<float4>(cli,f4Vec);
        cl_position_l = Buffer<float4>(cli, f4Vec);
        cl_veleval_u = Buffer<float4>(cli, f4Vec);
        cl_veleval_s = Buffer<float4>(cli, f4Vec);
        //cl_density_s = Buffer<float>(cli, densities);
        cl_rbParticleIndex = Buffer<int2>(cli,rbParticleIndex);
        cl_rbMass = Buffer<float>(cli,rbfVec);
        cl_comPos = Buffer<float4>(cli,comPosVBO);
        cl_comRot = Buffer<float4>(cli,comRotationVBO);
        cl_comVel = Buffer<float4>(cli,rbf4Vec);
        cl_comAngVel = Buffer<float4>(cli,rbf4Vec);
        cl_comVelEval = Buffer<float4>(cli,rbf4Vec);
        cl_comAngVelEval = Buffer<float4>(cli,rbf4Vec);
        cl_comLinearForce = Buffer<float4>(cli,rbf4Vec);
        cl_comTorqueForce = Buffer<float4>(cli,rbf4Vec);
        cl_invInertialTensor = Buffer<float16>(cli,rbf16Vec);
        rbParticleIndex.resize(0);
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
        dout<<"Number of Grid Cells "<< grid_params.nb_cells<<endl;
        vector<unsigned int> gcells(grid_params.nb_cells+1);
        int minus = 0xffffffff;
        std::fill(gcells.begin(), gcells.end(), 666);

        cl_cell_indices_start = Buffer<unsigned int>(cli, gcells);
        cl_cell_indices_end   = Buffer<unsigned int>(cli, gcells);
        cl_cell_static_indices_start = Buffer<unsigned int>(cli, gcells);
        cl_cell_static_indices_end   = Buffer<unsigned int>(cli, gcells);
        std::vector<unsigned int> keys(max_num);
        //to get around limits of bitonic sort only handling powers of 2
        std::fill(keys.begin(), keys.end(), INT_MAX);
        cl_sort_static_indices  = Buffer<unsigned int>(cli, keys);
        cl_sort_static_hashes   = Buffer<unsigned int>(cli, keys);

        // For bitonic sort. Remove when bitonic sort no longer used
        // Currently, there is an error in the Radix Sort (just run both
        // sorts and compare outputs visually
        cl_sort_static_output_hashes = Buffer<unsigned int>(cli, keys);
        cl_sort_static_output_indices = Buffer<unsigned int>(cli, keys);
     }

	//----------------------------------------------------------------------
    void ParticleRigidBody::pushParticles(vector<float4> pos, vector<float4> vels, float4 color, float mass)
    {
        //cut = 1;

        int nn = pos.size();
        if(nn==0)
        {
            cout<<"No particles to add."<<endl;
            return;
        }
        else if (num + nn > max_num)
        {
 	    cout<<"pushParticles: exceeded max nb("<<max_num<<") of particles allowed"<<endl;
            return;
        }

        vector<float4> cols(nn);

        fill(cols.begin(), cols.end(),color);

        //ifmass is 0.0f then the rigid-body is considered static and should not go in
        //the normal static array. This will save a lot of time because there is no need
        //to resort/hash static positions because they do not change.
        glFinish();
        if(mass!=0.0f)
        {
            //calculate center of mass.
            float4 com(0.0f,0.0f,0.0f,0.0f);
            for(int i = 0;i<pos.size();i++)
            {
                com+=pos[i];
            }
            com/=pos.size();
            com.w=1.0f;
            rbParticleIndex.push_back(int2(num,num+nn));
            vector<float4> pos_l;
            vector<float> mass_p;
            for(int i = 0;i<pos.size();i++)
            {
                float4 tmp = (pos[i]-com);
                tmp*=prbp.simulation_scale;
                tmp.w = 1.0f;
                pos_l.push_back(tmp);
                mass_p.push_back(mass/nn);
                //char tmpchar[32];
                //sprintf(tmpchar,"pos_l[%d]",i);
                //tmp.print(tmpchar);
            }
            //vector<float4> scaled_pos_l(pos_l.size());
            //for(int i = 0; i< pos_l.size();i++)
            //{
            //    scaled_pos_l[i]=pos_l[i]*prbp.simulation_scale;
            //}
            float16 invInertialTensor = calculateInvInertialTensor(pos_l,mass);
            float4 comVelEval=float4(0.0f,0.0f,0.0f,0.0f);
            float4 angMomentum =float4(0.0f,0.0f,0.0f,0.0f);
            for(int i = 0;i<vels.size();i++)
            {
               comVelEval=comVelEval+vels[i];
               float4 r = pos[i]-com;

               angMomentum.x+=r.y*vels[i].z-r.z*vels[i].y;
               angMomentum.y+=r.z*vels[i].x-r.x*vels[i].z;
               angMomentum.z+=r.x*vels[i].y-r.y*vels[i].x;
            }
            angMomentum*=mass_p[0];
            float4 comAngVelEval=float4(0.0f,0.0f,0.0f,0.0f);;
            comAngVelEval=invInertialTensor*angMomentum;

            dout<<"position: "<<com<<endl;
            dout<<"velocity: "<<comVelEval<<endl;
            dout<<"ang momentum: "<<angMomentum<<endl;
            dout<<"mass: "<<mass<<endl;
            dout<<"Inertial Tensor: "<<endl;
            for(int i =0;i<16;i++)
                cout<<i<<": "<<invInertialTensor.m[i]<<" ,";
            cout<<endl;
            vector<unsigned int> index(nn);
            std::fill(index.begin(), index.end(), curRigidbodyID);
            curRigidbodyID++;
    #ifdef GPU
            cl_position_u.acquire();
            cl_color_u.acquire();
            cl_velocity_u.acquire();
            cl_comPos.acquire();

            //printf("about to prep 0\n");
            //call_prep(0);
            //printf("done with prep 0\n");

            // Allocate max_num particles on the GPU. That wastes memory, but is useful.
            // There should be a way to update this during the simulation.
            cl_position_u.copyToDevice(pos, num);
            cl_color_u.copyToDevice(cols, num);
            cl_velocity_u.copyToDevice(vels, num);
            cl_position_l.copyToDevice(pos_l,num);
            cl_mass_u.copyToDevice(mass_p, num);
            cl_comVel.copyToDevice(comVelEval, rbParticleIndex.size()-1);
            cl_comVelEval.copyToDevice(comVelEval, rbParticleIndex.size()-1);
            cl_comAngVel.copyToDevice(comAngVelEval, rbParticleIndex.size()-1);
            cl_comAngVelEval.copyToDevice(comAngVelEval, rbParticleIndex.size()-1);
            cl_objectIndex_u.copyToDevice(index, num);
            /*cl_spring_coef_u.copyToDevice(spring_co, num);
            cl_dampening_coef_u.copyToDevice(dampening_co, num);*/
            vector<float4> comVec;
            comVec.push_back(com);
            cl_comPos.copyToDevice(comVec,rbParticleIndex.size()-1);
            vector<float> rbm;
            rbm.push_back(mass);
            cl_rbMass.copyToDevice(rbm,rbParticleIndex.size()-1);
            vector<float16> invInertial;
            invInertial.push_back(invInertialTensor);
            cl_invInertialTensor.copyToDevice(invInertial,rbParticleIndex.size()-1);
            //cl_rbParticleIndex.copyToDevice(rbParticleIndex,rbParticleIndex.size()-1,);
            cl_rbParticleIndex.copyToDevice(rbParticleIndex);
            //dout<<"particle index start = "<<rbParticleIndex.back().x <<" end = "<<rbParticleIndex.back().y<<endl;
            //dout<<"rbParticleIndex.size() = "<<rbParticleIndex.size()<<endl;

            settings->SetSetting("num_particles", num+nn);
            updateParams();

            cl_comPos.release();
            cl_position_u.release();
            cl_color_u.release();
            cl_velocity_u.release();
    #endif
            num += nn;  //keep track of number of particles we use
            dout<<"num = "<<num<<endl;
        }
        else
        {
            if (static_num + nn > max_num)
            {
                cout<<"pushParticles: exceeded max nb("<<max_num<<") of particles allowed"<<endl;
                return;
            }
            cl_static_position_u.acquire();
            cl_static_position_u.copyToDevice(pos, static_num);
            static_num+=nn;
            prepareStaticRBs();
            cl_static_position_u.release();
        }
        //renderer->setNum(num);
    }
	//----------------------------------------------------------------------
    /*void ParticleRigidBody::render()
    {
        renderer->render_box(grid->getBndMin(), grid->getBndMax());
        //renderer->render_table(grid->getBndMin(), grid->getBndMax());
        System::render();
    }*/

    void ParticleRigidBody::calculate()
    {
        //This shouldn't be here. We should just be able to set the rest distance
        float4 dmin = grid.getBndMin();
        float4 dmax = grid.getBndMax();
        spacing = settings->GetSettingAs<float>("smoothing_distance") /settings->GetSettingAs<float>("simulation_scale");
        settings->SetSetting("spacing", spacing);
        //constants
        settings->SetSetting("epsilon", 1E-6);
        settings->SetSetting("spring",(settings->GetSettingAs<float>("penetration_factor")*settings->GetSettingAs<float>("velocity_limit"))/(settings->GetSettingAs<float>("smoothing_distance")*settings->GetSettingAs<float>("smoothing_distance")));
        float ln_res =log(settings->GetSettingAs<float>("restitution"));
        settings->SetSetting("dampening",(2*-(ln_res))/sqrt((ln_res*ln_res)+(M_PI*M_PI)));

        //CL parameters
        settings->SetSetting("num_particles", 0);
    }



	//----------------------------------------------------------------------
    void ParticleRigidBody::updateParams()
    {
        //update all the members of the prbp struct
        prbp.smoothing_distance = settings->GetSettingAs<float>("smoothing_distance");
        prbp.simulation_scale = settings->GetSettingAs<float>("simulation_scale");

        //dynamic params
        prbp.gravity = settings->GetSettingAs<float4>("gravity"); // -9.8 m/sec^2
        prbp.friction_dynamic = settings->GetSettingAs<float>("friction_dynamic");
        prbp.friction_static = settings->GetSettingAs<float>("friction_static");
        prbp.friction_static_threshold = settings->GetSettingAs<float>("friction_static_threshold");
        prbp.dampening = settings->GetSettingAs<float>("dampening");
        //next 3 not used at the moment
        prbp.shear = settings->GetSettingAs<float>("shear");
        prbp.attraction = settings->GetSettingAs<float>("attraction");
        prbp.spring = settings->GetSettingAs<float>("spring");
        //prbp.surface_threshold;

        //constants
        prbp.EPSILON = settings->GetSettingAs<float>("epsilon");

        //CL parameters
        prbp.num = settings->GetSettingAs<int>("num_particles");
        prbp.max_num = settings->GetSettingAs<int>("max_num_particles");

        dout<<"smooth : "<<prbp.smoothing_distance<<endl;
        dout<<"scale : "<<prbp.simulation_scale<<endl;
        dout<<"gravity : "<<prbp.gravity<<endl;
        dout<<"dampening : "<<prbp.dampening<<endl;
        //update the OpenCL buffer
        //std::vector<SPHParams> vparams();
        //vparams.push_back(prbp);
        cl_prbp.copyToDevice(prbp,0);
        settings->updated();

    }
    void ParticleRigidBody::interact()
    {
            for(int j = 0;j<interactionSystem.size();j++)
            {
                //Naievely assume it is an sph system for now.
                //Need to come up with a good way to interact.
                timers["force_fluid"]->start();
                forceFluid.execute(   num,
                    //cl_vars_sorted,
                    cl_position_s,
                    cl_velocity_s,
                    cl_force_s,
                    cl_mass_s,
                    interactionSystem[j]->getPositionBuffer(),
                    interactionSystem[j]->getVelocityBuffer(),
                    interactionSystem[j]->getMassBuffer(),
                    cl_sort_indices,
                    interactionSystem[j]->getCellStartBuffer(),
                    interactionSystem[j]->getCellEndBuffer(),
                    cl_prbp,
                    //cl_GridParams,
                    cl_GridParamsScaled,
                    clf_debug,
                    cli_debug);
                timers["force_fluid"]->stop();
            }

            timers["segmented_scan"]->start();
            sscan.execute(num,
                    cl_position_l,
                    cl_rbParticleIndex,
                    cl_force_s,
                    cl_comLinearForce,
                    cl_comTorqueForce,
                    cl_comPos,
                    rbParticleIndex.size(),
                    cl_prbp,
                    //debug params
                    clf_debug,
                    cli_debug);
            timers["segmented_scan"]->stop();
    }
    void ParticleRigidBody::postProcess()
    {
           timers["update_particles"]->start();
            updateParticles.execute(rbParticleIndex.size(),
                    cl_position_u,
                    cl_position_l,
                    cl_velocity_u,
                    cl_rbParticleIndex,
                    cl_comPos,
                    cl_comRot,
                    cl_comVelEval,
                    cl_comAngVelEval,
                    cl_prbp,
                    //debug params
                    clf_debug,
                    cli_debug);
            timers["update_particles"]->start();
    }
    float16 ParticleRigidBody::calculateInvInertialTensor(vector<float4>& pos, float mass)
    {
        //FIXME: This is a very inefficient and ugly way for calculating the
        //invers of the inertial tensor.
        float a11=0.0f;
        float a12=0.0f;
        float a13=0.0f;
        float a21=0.0f;
        float a22=0.0f;
        float a23=0.0f;
        float a31=0.0f;
        float a32=0.0f;
        float a33=0.0f;
        for(int j = 0; j<pos.size(); j++)
        {
           a11 += (mass/pos.size()) * (sq(pos[j].y)+sq(pos[j].z));
        }
        for(int j = 0; j<pos.size(); j++)
        {
           a12 -= (mass/pos.size()) * (pos[j].x*pos[j].y);
        }
        a21 = a12;
        for(int j = 0; j<pos.size(); j++)
        {
           a13 -= (mass/pos.size()) * (pos[j].x*pos[j].z);
        }
        a31 = a13;
        for(int j = 0; j<pos.size(); j++)
        {
           a22 += (mass/pos.size()) * (sq(pos[j].x)+sq(pos[j].z));
        }
        for(int j = 0; j<pos.size(); j++)
        {
           a23 -= (mass/pos.size()) * (pos[j].y*pos[j].z);
        }
        a32 = a23;
        for(int j = 0; j<pos.size(); j++)
        {
           a33 += (mass/pos.size()) * (sq(pos[j].x)+sq(pos[j].y));
        }

        float det = 1.0f/(a11*(a33*a22-a32*a23)-a21*(a33*a12-a32*a13)+a31*(a23*a12-a22*a13));
        float16 invit;
        invit.m[0] =det*(a33*a22-a32*a23);
        invit.m[1] =-det*(a33*a12-a32*a13);
        invit.m[2] =det*(a23*a12-a22*a13);
        invit.m[3] = 0.0f;
        invit.m[4] =-det*(a33*a21-a31*a23);
        invit.m[5] =det*(a33*a11-a31*a13);
        invit.m[6] =-det*(a23*a11-a21*a13);
        invit.m[7] = 0.0f;
        invit.m[8] =det*(a32*a21-a31*a22);
        invit.m[9] =-det*(a32*a11-a31*a12);
        invit.m[10]=det*(a22*a11-a21*a12);
        invit.m[11] = 0.0f;
        invit.m[12] = 0.0f;
        invit.m[13] = 0.0f;
        invit.m[14] = 0.0f;
        invit.m[15] = 0.0f;

       return invit;
    }
    void ParticleRigidBody::prepareStaticRBs()
    {
        hash.execute(static_num,
                cl_static_position_u,
                cl_sort_static_hashes,
                cl_sort_static_indices,
                cl_GridParams,
                clf_debug,
                cli_debug);
        try
        {
            int dir = 1;        // dir: direction
            int arrayLength = nlpo2(static_num);
            int batch = 1;
            bitonic.Sort(batch,
                        arrayLength,
                        dir,
                        &cl_sort_static_output_hashes,
                        &cl_sort_static_output_indices,
                        &cl_sort_static_hashes,
                        &cl_sort_static_indices );

        }
        catch (cl::Error er)
        {
            cout<<"ERROR(bitonic sort): "<< er.what()<<"("<< CL::oclErrorString(er.err())<<")"<<endl;
        }

             timers["cellindices"]->start();
            int nc = cellindices.execute(static_num,
                cl_sort_static_hashes,
                cl_sort_static_indices,
                cl_cell_static_indices_start,
                cl_cell_static_indices_end,
                cl_GridParams,
                grid_params.nb_cells,
                clf_debug,
                cli_debug);
            timers["cellindices"]->stop();

			//-----------------
            timers["permute"]->start();
            permute.execute(static_num,
                cl_static_position_u,
                cl_static_position_s,
                cl_sort_static_indices,
                clf_debug,
                cli_debug);
            timers["permute"]->stop();
           cli->queue.finish();
    }
    void ParticleRigidBody::acquireGLBuffers()
    {
        cl_comPos.acquire();
        cl_comRot.acquire();
        System::acquireGLBuffers();
    }
    void ParticleRigidBody::releaseGLBuffers()
    {
        cl_comPos.release();
        cl_comRot.release();
        System::releaseGLBuffers();
    }
}; //end namespace
