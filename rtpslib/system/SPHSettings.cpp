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


//#include <SPHSettings.h>
#include <SPH.h>

namespace rtps
{
    void SPH::calculate()
    {
        /*!
        * The Particle Mass (and hence everything following) depends on the MAXIMUM number of particles in the system
        */

        float rho0 = settings->GetSettingAs<float>("rest_density");                              //rest density [kg/m^3 ]
        //float mass = (128*1024.0)/max_num * .0002;    //krog's way
        //float VP = (1.0/rho0)*0.00002*max_num;
        //float VP = 1.0/max_num;
        //float mass = (rho0)/max_num;
        //float VP = 2 * .0262144 / max_num;            //Particle Volume [ m^3 ]
        //float VP = .0262144 / 16000;                  //Particle Volume [ m^3 ]
        float mass = (0.0256/(int)log2(settings->GetSettingAs<unsigned int>("max_num_particles"));         //Particle Mass [ kg ]
        float VP = mass/rho0;
        //float mass = (rho0*VP/max_num);
        //constant .87 is magic
        //float rest_distance = 0.005;
        float rest_distance = .87 * pow(mass/rho0, 1.f/3.f);   //rest distance between particles [ m ]
        //float rest_distance = pow(VP, 1.f/3.f);     //rest distance between particles [ m ]
        float smoothing_distance = 2.0f * rest_distance;//interaction radius


        float4 dmin = grid->getBndMin();
        float4 dmax = grid->getBndMax();
        //printf("dmin: %f %f %f\n", dmin.x, dmin.y, dmin.z);
        //printf("dmax: %f %f %f\n", dmax.x, dmax.y, dmax.z);
        float domain_vol = (dmax.x - dmin.x) * (dmax.y - dmin.y) * (dmax.z - dmin.z);
        //printf("domain volume: %f\n", domain_vol);


        //ratio between particle radius in simulation coords and world coords
        //float simulation_scale = pow(1./ domain_vol,1.f/3.f); 
        float simulation_scale = pow(.5f * VP * max_num/ domain_vol, 1.f/3.f); 
        //float simulation_scale = pow(VP * 16000/ domain_vol, 1.f/3.f); 

		//int max_cloud_num = cloud->getMaxCloudNum();

		// Cloud update (SHOULD NOT BE REQUIRED
        //settings->SetSetting("Maximum Number of Cloud Particles", max_cloud_num);
       
        settings->SetSetting("mass", mass);
        settings->SetSetting("rest_distance", rest_distance);
        settings->SetSetting("smoothing_distance", smoothing_distance);
        settings->SetSetting("simulation_scale", simulation_scale);

		// Why did Ian choose the 2nd line
        float boundary_distance = .5f * rest_distance;
        //float boundary_distance =  smoothing_distance;

        settings->SetSetting("boundary_distance", boundary_distance);
        float spacing = (rest_distance / simulation_scale);
        //float spacing = smoothing_distance / simulation_scale;
        settings->SetSetting("spacing", spacing);

        float pi = M_PI;
        float h9 = pow(smoothing_distance, 9.f);
        float h6 = pow(smoothing_distance, 6.f);
        float h3 = pow(smoothing_distance, 3.f);
        //Kernel Coefficients
        settings->SetSetting("wpoly6", 315.f/64.0f/pi/h9 );
        settings->SetSetting("wpoly6_d", -945.f/(32.0f*pi*h9) );  //doesn't seem right
        settings->SetSetting("wpoly6_dd", -945.f/(32.0f*pi*h9) ); // laplacian
        settings->SetSetting("wspiky", 15.f/pi/h6 );
        settings->SetSetting("wspiky_d", -45.f/(pi*h6) );
        settings->SetSetting("wspiky_dd", 15./(2.*pi*h3) );
        settings->SetSetting("wvisc", 15./(2.*pi*h3) );
        settings->SetSetting("wvisc_d", 15./(2.*pi*h3) ); //doesn't seem right
        settings->SetSetting("wvisc_dd", 45./(pi*h6) );

        //dynamic params
        settings->SetSetting("gravity", float4(0.0f,0.0f,-9.8f,0.0f); // -9.8 m/sec^2
        settings->SetSetting("gas_constant", 1.5f);
        settings->SetSetting("viscosity", 1.0f);
        settings->SetSetting("velocity_limit", 600.0f);
        settings->SetSetting("xsph_factor", .1f);
        settings->SetSetting("friction_kinetic", 0.2f);
        settings->SetSetting("friction_static", 0.0f);
        settings->SetSetting("boundary_stiffness", 20000.0f);
        settings->SetSetting("boundary_dampening", 256.0f);


        //next 4 not used at the moment
        settings->SetSetting("restitution", 0.0f);
        settings->SetSetting("shear", 0.0f);
        settings->SetSetting("attraction", 0.0f);
        settings->SetSetting("spring", 0.0f);

        //constants
        settings->SetSetting("epsilon", 1E-6);

        //CL parameters
        settings->SetSetting("num_particles", 0);
    }
   


	//----------------------------------------------------------------------
    void SPH::updateParams()
    {

        //update all the members of the sphp struct
        sphp.mass = settings->GetSettingAs<float>("mass");
        sphp.rest_distance = settings->GetSettingAs<float>("rest_distance");
        sphp.smoothing_distance = settings->GetSettingAs<float>("smoothing_distance");
        sphp.simulation_scale = settings->GetSettingAs<float>("simulation_scale");
        
        //dynamic params
        sphp.boundary_stiffness = settings->GetSettingAs<float>("boundary_stiffness");
        sphp.boundary_dampening = settings->GetSettingAs<float>("boundary_dampening");
        sphp.boundary_distance = settings->GetSettingAs<float>("boundary_distance");
        sphp.K = settings->GetSettingAs<float>("gas_constant");        //gas constant
        sphp.viscosity = settings->GetSettingAs<float>("viscosity");
        sphp.velocity_limit = settings->GetSettingAs<float>("velocity_limit");
        sphp.xsph_factor = settings->GetSettingAs<float>("xsph_factor");
        sphp.gravity = settings->GetSettingAs<float4>("gravity"); // -9.8 m/sec^2
        sphp.friction_coef = settings->GetSettingAs<float>("friction");
        sphp.restitution_coef = settings->GetSettingAs<float>("restitution");
        //next 3 not used at the moment
        sphp.shear = settings->GetSettingAs<float>("shear");
        sphp.attraction = settings->GetSettingAs<float>("attraction");
        sphp.spring = settings->GetSettingAs<float>("spring");
        //sphp.surface_threshold;

        //constants
        sphp.EPSILON = settings->GetSettingAs<float>("epsilon");
        //Kernel Coefficients
        sphp.wpoly6_coef = settings->GetSettingAs<float>("wpoly6");
        sphp.wpoly6_d_coef = settings->GetSettingAs<float>("wpoly6_d");
        sphp.wpoly6_dd_coef = settings->GetSettingAs<float>("wpoly6_dd"); // laplacian
        sphp.wspiky_coef = settings->GetSettingAs<float>("wspiky");
        sphp.wspiky_d_coef = settings->GetSettingAs<float>("wspiky_d");
        sphp.wspiky_dd_coef = settings->GetSettingAs<float>("wspiky_dd");
        sphp.wvisc_coef = settings->GetSettingAs<float>("wvisc");
        sphp.wvisc_d_coef = settings->GetSettingAs<float>("wvisc_d");
        sphp.wvisc_dd_coef = settings->GetSettingAs<float>("wvisc_dd");

        //CL parameters
        sphp.num = settings->GetSettingAs<int>("num_particles");
        sphp.max_num = settings->GetSettingAs<int>("max_num_particles");

        //update the OpenCL buffer
        //std::vector<SPHParams> vparams();
        //vparams.push_back(sphp);
        cl_sphp.copyToDevice(vparams,0);
        settings->updated();
    }
}
