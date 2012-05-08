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


#ifndef RTPS_SPHSETTINGS_H_INCLUDED
#define RTPS_SPHSETTINGS_H_INCLUDED

#include "../structs.h"

namespace rtps
{

#ifdef WIN32
#pragma pack(push,16)
#endif

    //Struct which gets passed to OpenCL routines
	typedef struct SPHParams
    {
        float rest_density;
        float mass;
        float rest_distance;
        float smoothing_distance;
        float simulation_scale;
        
        //dynamic params
        float boundary_stiffness;
        float boundary_dampening;
        float boundary_distance;
        float K;        //gas constant
        
        float viscosity;
        float velocity_limit;
        float xsph_factor;

        float friction_coef;
        float restitution_coef;
        float shear;
        float attraction;
        float spring;

        //constants
        float EPSILON;

        //Kernel Coefficients
        float wpoly6_coef;
        float wpoly6_d_coef;
        float wpoly6_dd_coef; // laplacian
        float wspiky_coef;
        float wspiky_d_coef;

        float wspiky_dd_coef;
        float wvisc_coef;
        float wvisc_d_coef;
        float wvisc_dd_coef;

        //CL parameters
        int num;
        int max_num;

        float4 gravity;
		//float padding[3]; //using this because padding doesn't seem to work correctly in windows
		// -9.8 m/sec^2
        
        void print()
        {
			std::cout<<"rest_density = "<<rest_density<<std::endl;
        std::cout<<"mass = "<<mass<<std::endl;
        std::cout<<"rest_distance = "<<rest_distance<<std::endl;
        std::cout<<"smoothing_distance = "<<smoothing_distance<<std::endl;
        std::cout<<"simulation_scale = "<<simulation_scale<<std::endl;
        
        //dynamic params
        std::cout<<"boundary_stiffness = "<<boundary_stiffness<<std::endl;
        std::cout<<"boundary_dampening = "<<boundary_dampening<<std::endl;
        std::cout<<"boundary_distance = "<<boundary_distance<<std::endl;
        std::cout<<"K = "<<K<<std::endl;        //gas constant
        
        std::cout<<"viscosity = "<<viscosity<<std::endl;
        std::cout<<"velocity_limit = "<<velocity_limit<<std::endl;
        std::cout<<"xpsh_factor = "<<xsph_factor<<std::endl;

        std::cout<<"friction_coef = "<<friction_coef<<std::endl;
        std::cout<<"restitution_coef = "<<restitution_coef<<std::endl;
        //float shear;
        std::cout<<"attraction = "<<attraction<<std::endl;
        std::cout<<"spring = "<<spring<<std::endl;

        //constants
        std::cout<<"epsilon = "<<EPSILON<<std::endl;

        //Kernel Coefficients
        std::cout<<"wpoly6_coef = "<<wpoly6_coef<<std::endl;
        std::cout<<"wpoly6_d_coef = "<<wpoly6_d_coef<<std::endl;
        std::cout<<"wpoly6_dd_coef = "<<wpoly6_dd_coef<<std::endl; // laplacian
        std::cout<<"wspiky_coef = "<<wspiky_coef<<std::endl;
        std::cout<<"wspiky_d_coef = "<<wspiky_d_coef<<std::endl;

        std::cout<<"wspiky_dd_coef = "<<wspiky_dd_coef<<std::endl;
        std::cout<<"wvisc_coef = "<<wvisc_coef<<std::endl;
        std::cout<<"wvisc_d_coef = "<<wvisc_d_coef<<std::endl;
        std::cout<<"wvisc_dd_coef = "<<wvisc_dd_coef<<std::endl;

        //CL parameters
        std::cout<<"num = "<<num<<std::endl;
        std::cout<<"max_num = "<<max_num<<std::endl;

        std::cout<<"gravity = "<<gravity<<std::endl;
        }
    } SPHParams
#ifndef WIN32
	__attribute__((aligned(16)));
#else
		;
        #pragma pack(pop)
#endif
}
#endif
