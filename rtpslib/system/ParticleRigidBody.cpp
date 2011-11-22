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
    ParticleRigidBody::ParticleRigidBody(RTPS *psfr, int n) :System(psfr,n)
    {
        resource_path = settings->GetSettingAs<string>("rtps_path");
        printf("resource path: %s\n", resource_path.c_str());

        std::vector<ParticleRigidBodyParams> vparams(0);
        vparams.push_back(prbp);
        cl_prbp = Buffer<ParticleRigidBodyParams>(ps->cli, vparams);

        calculate();
        updateParams();

        //settings->printSettings();

        spacing = settings->GetSettingAs<float>("Spacing");

        printf("Spacing class = %f\n",spacing);
        //ParticleRigidBody settings depend on number of particles used
        //calculateParticleRigidBodySettings();
        //set up the grid
        setupDomain(prbp.smoothing_distance/prbp.simulation_scale,prbp.simulation_scale);

        //integrator = LEAPFROG;
        integrator = EULER;


        //*** end Initialization

        setupTimers();

#ifdef GPU
        printf("RUNNING ON THE GPU\n");
        
        //setup the sorted and unsorted arrays
        prepareSorted();

        //should be more cross platform
        rigidbody_source_dir = resource_path + "/" + std::string(PARTICLE_RIGIDBODY_CL_SOURCE_DIR);
        ps->cli->addIncludeDir(rigidbody_source_dir);
        force = PRBForce(rigidbody_source_dir, ps->cli, timers["force_gpu"]);
        forceFluid = PRBForceFluid(rigidbody_source_dir, ps->cli, timers["force_fluid_gpu"]);
        sscan = PRBSegmentedScan(rigidbody_source_dir, ps->cli, timers["segmented_scan_gpu"]);
        updateParticles = PRBUpdateParticles(rigidbody_source_dir, ps->cli, timers["update_particles_gpu"]);
		

        //could generalize this to other integration methods later (leap frog, RK4)
        if (integrator == LEAPFROG)
        {
            //loadLeapFrog();
            leapfrog = PRBLeapFrog(rigidbody_source_dir, ps->cli, timers["leapfrog_gpu"]);
        }
        else if (integrator == EULER)
        {
            //loadEuler();
            euler = PRBEuler(rigidbody_source_dir, ps->cli, timers["euler_gpu"]);
        }

#endif

        renderer->setParticleRadius(spacing);
    }

	//----------------------------------------------------------------------
    ParticleRigidBody::~ParticleRigidBody()
    {
        printf("Particle Rigid Body Destructor\n");
    }

	//----------------------------------------------------------------------
    void ParticleRigidBody::update()
    {
		//printf("+++++++++++++ enter UPDATE()\n");
        //call kernels
#ifdef GPU
        updateGPU();
#endif
    }

	//----------------------------------------------------------------------
    void ParticleRigidBody::updateGPU()
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
                cl_GridParams,
                grid_params.nb_cells,
                clf_debug,
                cli_debug);
            timers["cellindices"]->stop();

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
                renderer->setNum(prbp.num);

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
            //FIXME: Needs to calculate linear force and torque forces.
            timers["force"]->start();
            force.execute(   num,
                //cl_vars_sorted,
                cl_position_s,
                cl_velocity_s,
                //cl_veleval_s,
                cl_force_s,
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
        }


        timers["update"]->stop();
    }

    void ParticleRigidBody::integrate()
    {
        timers["integrate"]->start();
        if (integrator == EULER)
        {
            //euler();
            euler.execute(num,
                settings->dt,
                cl_comLinearForce,
                cl_comTorqueForce,
                cl_comVel,
                cl_comAngVel,
                cl_comPos,
                cl_comRot,
                cl_invInertialTensor,
                cl_rbMass,
                    float4(0.0,0.0,prbp.gravity,0.0),
                rbParticleIndex.size(),
                //debug
                cl_prbp,
                clf_debug,
                cli_debug);


        }
        else if (integrator == LEAPFROG)
        {
            //leapfrog();
            /*leapfrog.execute(num,
                settings->dt,
                cl_position_u,
                cl_position_s,
                cl_velocity_u,
                cl_velocity_s,
                cl_veleval_u,
                cl_linear_force_u,
                //cl_vars_unsorted, 
                //cl_vars_sorted, 
                cl_sort_indices,
                cl_prbp,
                //debug
                clf_debug,
                cli_debug);
             */
        }

		// Perhaps I am messed up by Courant condition if cloud point 
		// velocities are too large? 

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

        std::fill(f4Vec.begin(), f4Vec.end(), float4(0.0f, 0.0f, 0.0f, 0.0f));
        std::fill(rbf4Vec.begin(), rbf4Vec.end(), float4(0.0f, 0.0f, 0.0f, 0.0f));
        std::fill(rbf16Vec.begin(), rbf16Vec.end(), (float16){0.0f});
        std::fill(rotf4Vec.begin(), rotf4Vec.end(), float4(0.0f, 0.0f, 0.0f, 1.0f));
        std::fill(rbfVec.begin(), rbfVec.end(),0.0f);
        std::fill(rbParticleIndex.begin(),rbParticleIndex.end(),int2(0,0));

        cl_position_l = Buffer<float4>(ps->cli, f4Vec);
        cl_veleval_u = Buffer<float4>(ps->cli, f4Vec);
        cl_veleval_s = Buffer<float4>(ps->cli, f4Vec);
        //cl_density_s = Buffer<float>(ps->cli, densities);
        cl_rbParticleIndex = Buffer<int2>(ps->cli,rbParticleIndex);
        cl_rbMass = Buffer<float>(ps->cli,rbfVec);
        cl_comPos = Buffer<float4>(ps->cli,rbf4Vec);
        cl_comRot = Buffer<float4>(ps->cli,rotf4Vec);
        cl_comVel = Buffer<float4>(ps->cli,rbf4Vec);
        cl_comAngVel = Buffer<float4>(ps->cli,rbf4Vec);
        cl_comLinearForce = Buffer<float4>(ps->cli,rbf4Vec);
        cl_comTorqueForce = Buffer<float4>(ps->cli,rbf4Vec);
        cl_invInertialTensor = Buffer<float16>(ps->cli,rbf16Vec);
        rbParticleIndex.resize(0);
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
    void ParticleRigidBody::pushParticles(vector<float4> pos, vector<float4> vels, float4 color, float mass)
    {
        //cut = 1;

        int nn = pos.size();
        if (num + nn > max_num)
        {
			printf("pushParticles: exceeded max nb(%d) of particles allowed\n", max_num);
            return;
        }
        //float rr = (rand() % 255)/255.0f;
        //float4 color(rr, 0.0f, 1.0f - rr, 1.0f);
        //printf("random: %f\n", rr);
        //float4 color(1.0f,1.0f,1.0f,1.0f);

        std::vector<float4> cols(nn);
        //printf("color: %f %f %f %f\n", color.x, color.y, color.z, color.w);

        std::fill(cols.begin(), cols.end(),color);
        //float v = .5f;
        //float v = 0.0f;
        //float4 iv = float4(v, v, -v, 0.0f);
        //float4 iv = float4(0, v, -.1, 0.0f);
        //std::fill(vels.begin(), vels.end(),iv);

        float spring = settings->GetSettingAs<float>("Penetration Factor")*settings->GetSettingAs<float>("Velocity Limit")*(mass/nn)/prbp.smoothing_distance;
        float ln_res =log(settings->GetSettingAs<float>("Restitution")); 
        
        float dampening = -2.*ln_res*(sqrt((spring*(mass/nn))/((ln_res*ln_res)+(M_PI*M_PI))));
        //spring = -spring;
        std::vector<float> spring_co(nn);
        std::vector<float> dampening_co(nn);
        std::fill(spring_co.begin(), spring_co.end(),spring);
        std::fill(dampening_co.begin(), dampening_co.end(),dampening);

        printf("spring = %f\n",spring);
        printf("dampening = %f\n",dampening);
        printf("restitution = %f\n",settings->GetSettingAs<float>("Restitution"));
        settings->SetSetting("Boundary Stiffness", spring);
        settings->SetSetting("Boundary Dampening", dampening);
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
        for(int i = 0;i<pos.size();i++)
        {
            float4 tmp = pos[i]-com;
            tmp.w = 1.0f;
            pos_l.push_back(tmp);
            //char tmpchar[32];
            //sprintf(tmpchar,"pos_l[%d]",i);
            //tmp.print(tmpchar);
        }
        float16 invInertialTensor = calculateInvInertialTensor(pos,mass);
        
#ifdef GPU
        glFinish();
        cl_position_u.acquire();
        cl_color_u.acquire();
        cl_velocity_u.acquire();

        //printf("about to prep 0\n");
        //call_prep(0);
        //printf("done with prep 0\n");

		// Allocate max_num particles on the GPU. That wastes memory, but is useful. 
		// There should be a way to update this during the simulation. 
        cl_position_u.copyToDevice(pos, num);
        cl_color_u.copyToDevice(cols, num);
        cl_velocity_u.copyToDevice(vels, num);
        cl_position_l.copyToDevice(pos_l,num);
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
        com.print("Center of Mass");
        printf("particle index start = %d end = %d\n",rbParticleIndex.back().x,rbParticleIndex.back().y);
        printf("rbParticleIndex.size() = %d\n",rbParticleIndex.size());

        settings->SetSetting("Number of Particles", num+nn);
        updateParams();

        cl_position_u.release();
        cl_color_u.release();
        cl_velocity_u.release();
#endif
        num += nn;  //keep track of number of particles we use
        printf("num = %d\n",num);
        renderer->setNum(num);
    }
	//----------------------------------------------------------------------
    void ParticleRigidBody::render()
    {
        renderer->render_box(grid->getBndMin(), grid->getBndMax());
        //renderer->render_table(grid->getBndMin(), grid->getBndMax());
        System::render();
    }

    void ParticleRigidBody::calculate()
    {
        /*!
        * The Particle Mass (and hence everything following) depends on the MAXIMUM number of particles in the system
        */

        float rho0 = 1000;                              //rest density [kg/m^3 ]
        //float mass = (128*1024.0)/max_num * .0002;    //krog's way
        float VP = 2 * .0262144 / max_num;              //Particle Volume [ m^3 ]
        //float VP = .0262144 / 16000;                  //Particle Volume [ m^3 ]
        float mass = rho0 * VP;                         //Particle Mass [ kg ]
        //constant .87 is magic
        float rest_distance = .87 * pow(VP, 1.f/3.f);   //rest distance between particles [ m ]
        //float rest_distance = pow(VP, 1.f/3.f);     //rest distance between particles [ m ]
        float smoothing_distance = 2.0f * rest_distance;//interaction radius


        float4 dmin = grid->getBndMin();
        float4 dmax = grid->getBndMax();
        //printf("dmin: %f %f %f\n", dmin.x, dmin.y, dmin.z);
        //printf("dmax: %f %f %f\n", dmax.x, dmax.y, dmax.z);
        float domain_vol = (dmax.x - dmin.x) * (dmax.y - dmin.y) * (dmax.z - dmin.z);
        //printf("domain volume: %f\n", domain_vol);


        //ratio between particle radius in simulation coords and world coords
        float simulation_scale = pow(.5f * VP * max_num / domain_vol, 1.f/3.f); 
        //float simulation_scale = pow(VP * 16000/ domain_vol, 1.f/3.f); 

		//int max_cloud_num = cloud->getMaxCloudNum();

		// Cloud update (SHOULD NOT BE REQUIRED
        //settings->SetSetting("Maximum Number of Cloud Particles", max_cloud_num);
       
        settings->SetSetting("Maximum Number of Particles", max_num);
        settings->SetSetting("Mass", mass);
        settings->SetSetting("Rest Distance", rest_distance);
        settings->SetSetting("Smoothing Distance", smoothing_distance);
        settings->SetSetting("Simulation Scale", simulation_scale);


		// Why did Ian choose the 2nd line
        float boundary_distance = .5f * rest_distance;
        //float boundary_distance =  smoothing_distance;

        settings->SetSetting("Boundary Distance", boundary_distance);
        //float spacing = rest_distance / simulation_scale;
        float spacing = 2.f*(smoothing_distance / simulation_scale);
        printf("Spacing = %f\n",spacing);
        settings->SetSetting("Spacing", spacing);
 

        if(!settings->Exists("Gravity"))
            settings->SetSetting("Gravity", -9.8f); // -9.8 m/sec^2
        settings->SetSetting("Gas Constant", 15.0f);
        settings->SetSetting("Viscosity", .01f);
        settings->SetSetting("Velocity Limit", 600.0f);
        settings->SetSetting("XParticleRigidBody Factor", .1f);
        settings->SetSetting("Friction Kinetic", 0.0f);
        settings->SetSetting("Friction Static", 0.0f);

        //constants
        settings->SetSetting("EPSILON", 1E-6);
        settings->SetSetting("PI", M_PI);       //delicious

        //CL parameters
        settings->SetSetting("Number of Particles", 0);
        settings->SetSetting("Number of Variables", 10); // for combined variables (vars_sorted, etc.) //TO be depracated
    }
   


	//----------------------------------------------------------------------
    void ParticleRigidBody::updateParams()
    {

        //update all the members of the prbp struct
        //prbp.grid_min = this->settings->GetSettingAs<float4>; //settings->GetSettingAs doesn't support float4
        //prbp.grid_max;
        prbp.mass = settings->GetSettingAs<float>("Mass");
        prbp.rest_distance = settings->GetSettingAs<float>("Rest Distance");
        prbp.smoothing_distance = settings->GetSettingAs<float>("Smoothing Distance");
        prbp.simulation_scale = settings->GetSettingAs<float>("Simulation Scale");

		printf("prbp.simulation_scale= %f\n", prbp.simulation_scale);
		printf("prbp.smoothing_distance= %f\n", prbp.smoothing_distance);
        
        //dynamic params
        prbp.boundary_stiffness = settings->GetSettingAs<float>("Boundary Stiffness");
        prbp.boundary_dampening = settings->GetSettingAs<float>("Boundary Dampening");
        prbp.boundary_distance = settings->GetSettingAs<float>("Boundary Distance");
        //prbp.K = settings->GetSettingAs<float>("Gas Constant");        //gas constant
        //prbp.viscosity = settings->GetSettingAs<float>("Viscosity");
        //prbp.velocity_limit = settings->GetSettingAs<float>("Velocity Limit");
        //prbp.xsph_factor = settings->GetSettingAs<float>("XParticleRigidBody Factor");
        prbp.gravity = settings->GetSettingAs<float>("Gravity"); // -9.8 m/sec^2
        prbp.friction_coef = settings->GetSettingAs<float>("Friction");
        prbp.restitution_coef = settings->GetSettingAs<float>("Restitution");

        //next 3 not used at the moment
        prbp.shear = settings->GetSettingAs<float>("Shear");
        prbp.attraction = settings->GetSettingAs<float>("Attraction");
        prbp.spring = settings->GetSettingAs<float>("Spring");
        //prbp.surface_threshold;

        //constants
        prbp.EPSILON = settings->GetSettingAs<float>("EPSILON");
        prbp.PI = settings->GetSettingAs<float>("PI");       //delicious
        //Kernel Coefficients
        prbp.wpoly6_coef = settings->GetSettingAs<float>("wpoly6");
        prbp.wpoly6_d_coef = settings->GetSettingAs<float>("wpoly6_d");
        prbp.wpoly6_dd_coef = settings->GetSettingAs<float>("wpoly6_dd"); // laplacian
        prbp.wspiky_coef = settings->GetSettingAs<float>("wspiky");
        prbp.wspiky_d_coef = settings->GetSettingAs<float>("wspiky_d");
        prbp.wspiky_dd_coef = settings->GetSettingAs<float>("wspiky_dd");
        prbp.wvisc_coef = settings->GetSettingAs<float>("wvisc");
        prbp.wvisc_d_coef = settings->GetSettingAs<float>("wvisc_d");
        prbp.wvisc_dd_coef = settings->GetSettingAs<float>("wvisc_dd");

        //CL parameters
        prbp.num = settings->GetSettingAs<int>("Number of Particles");
        prbp.nb_vars = settings->GetSettingAs<int>("Number of Variables"); // for combined variables (vars_sorted, etc.)
        prbp.choice = settings->GetSettingAs<int>("Choice"); // which kind of calculation to invoke
        prbp.max_num = settings->GetSettingAs<int>("Maximum Number of Particles");

        //update the OpenCL buffer
        std::vector<ParticleRigidBodyParams> vparams(0);
        vparams.push_back(prbp);
        cl_prbp.copyToDevice(vparams);

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
                    interactionSystem[j]->getPositionBuffer(),
                    interactionSystem[j]->getVelocityBuffer(),
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
                    cl_position_u,
                    cl_rbParticleIndex,
                    cl_force_s,
                    cl_comLinearForce,
                    cl_comTorqueForce,
                    cl_comPos,
                    rbParticleIndex.size(),
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
                    cl_comVel,
                    cl_comAngVel,
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
}; //end namespace
