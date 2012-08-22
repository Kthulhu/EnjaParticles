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


#ifndef RTPS_PARTICLERIGIDBODYPARAMS_H_INCLUDED
#define RTPS_PARTICLERIGIDBODYPARAMS_H_INCLUDED

#include "../structs.h"
#include "../opencl/Buffer.h"
#include "../domain/Domain.h"

namespace rtps
{

#ifdef WIN32
#pragma pack(push,16)
#endif

    //Struct which gets passed to OpenCL routines
	typedef struct ParticleRigidBodyParams
    {
        float smoothing_distance;
        float simulation_scale;

        //dynamic params
        float4 gravity; // -9.8 m/sec^2

        float friction_dynamic;
        float friction_static;
        float friction_static_threshold;
        float shear;
        float attraction;
        float spring;
        float dampening;
        //constants
        float EPSILON;
        float velocity_limit;

        //CL parameters
        int num;
        int max_num;

        /*void print()
        {
            printf("----- ParticleRigidBodyParams ----\n");
            printf("mass: %f\n", mass);
            printf("rest distance: %f\n", rest_distance);
            printf("smoothing distance: %f\n", smoothing_distance);
            printf("simulation_scale: %f\n", simulation_scale);
            printf("--------------------\n");

            printf("friction_coef: %f\n", friction_coef);
            printf("restitution_coef: %f\n", restitution_coef);
            printf("damping: %f\n", boundary_dampening);
            printf("shear: %f\n", shear);
            printf("attraction: %f\n", attraction);
            printf("spring: %f\n", spring);
            printf("gravity: %f\n", gravity);
            printf("choice: %d\n", choice);
        }*/
    } ParticleRigidBodyParams
#ifndef WIN32
	__attribute__((aligned(16)));
#else
		;
        #pragma pack(pop)
#endif
}

#endif
