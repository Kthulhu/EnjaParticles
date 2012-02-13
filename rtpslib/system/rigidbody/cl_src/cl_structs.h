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


#ifndef _CL_PARTICLE_RIGIDBODY_STRUCTURES_H_
#define _CL_PARTICLE_RIGIDBODY_STRUCTURES_H_

#include "../cl_common/cl_structs.h"

//Struct which gets passed to OpenCL routines
typedef struct ParticleRigidBodyParams
{
    float smoothing_distance;
    float simulation_scale;

    //dynamic params
    float4 gravity; // -9.8 m/sec^2

    float friction_coef;
    float restitution_coef;
    float dampening_denom;
    float shear;
    float attraction;
    float spring;
    //constants
    float EPSILON;

    float static_stiffness;
    //CL parameters
    int num;
    int max_num;

} ParticleRigidBodyParams;



// Will be local variable
// used to output multiple variables per point
typedef struct PointData
{
    // density.x: density
    // density.y: denominator: sum_i (m_j/rho_j W_j)
    float4 density;
    float4 color;  // x component
    float4 color_normal;
    float4 color_lapl;
    float4 linear_force;
    float4 torque_force;
    //	float4 center_of_mass;
    //	int num_neighbors;
} PointData;



#endif
