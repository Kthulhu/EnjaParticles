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


#ifndef RTPS_PARTICLERIGIDBODY_H_INCLUDED
#define RTPS_PARTICLERIGIDBODY_H_INCLUDED

#ifdef WIN32
#define _USE_MATH_DEFINES
#include <cmath>
#endif

#include <string>

#include <RTPS.h>
#include <System.h>
#include <Kernel.h>
#include <Buffer.h>

#include <Domain.h>
#include "ParticleRigidBodyParams.h"

#include <util.h>

#include <rigidbody/PRBLeapFrog.h>
#include <rigidbody/PRBEuler.h>
#include <rigidbody/PRBForce.h>
#include <rigidbody/PRBForceFluid.h>
#include <rigidbody/PRBSegmentedScan.h>
#include <rigidbody/PRBUpdateParticles.h>
#include <structs.h>

//FIXME:needed for Integrator definition. This should be fixed and made more generic.
#include <SPHSettings.h>
#include <timer_eb.h>

#ifdef WIN32
    #if defined(rtps_EXPORTS)
        #define RTPS_EXPORT __declspec(dllexport)
    #else
        #define RTPS_EXPORT __declspec(dllimport)
	#endif 
#else
    #define RTPS_EXPORT
#endif
namespace rtps
{
    class RTPS_EXPORT ParticleRigidBody : public System
    {
    public:
        ParticleRigidBody(RTPS *ps, int num);
        ~ParticleRigidBody();

        void update();
        void interact();
        void integrate();
        void postProcess();

//        virtual void render();

        //void loadTriangles(std::vector<Triangle> &triangles);

        //void testDelete();
        //int cut; //for debugging DEBUG

        int setupTimers();
        void pushParticles(vector<float4> pos, vector<float4> velo, float4 color=float4(1.0, 0.0, 0.0, 1.0), float mass = 0.0);

        std::vector<float4> getDeletedPos();
        std::vector<float4> getDeletedVel();

    private:
       
        ParticleRigidBodyParams prbp;
        Integrator integrator;

        std::string rigidbody_source_dir;

        std::vector<float4> deleted_pos;
        std::vector<float4> deleted_vel;

        void prepareSorted();

        /*std::vector<float4> veleval;

        std::vector<float4> linearForce;
        std::vector<float4> torqueForce;*/
        
        //Maps a string name to an index in the following arrays
        //That allows for convenient lookup of rigid bodies. 
        std::map<std::string,int> rbIndex;
        std::vector<int2> rbParticleIndex;//first int is starting index. Second int is end index.
        /*std::vector<float4> comPos;
        std::vector<float4> comRot;*/

        //FIXME: Should find a more efficient way of doing this. For now
        //I will have 3 position buffers for particles. One holds unsorted global
        //positions, another holds sorted global positions, and a third holds 
        //unsorted local positions(per_rigid_body).
        Buffer<float4>      cl_position_l;
        Buffer<float4>      cl_veleval_u;
        Buffer<float4>      cl_veleval_s;

        Buffer<int2> cl_rbParticleIndex;
        Buffer<float> cl_rbMass;
        Buffer<float4> cl_comPos;
        Buffer<float4> cl_comRot;
        Buffer<float4> cl_comVel;
        Buffer<float4> cl_comAngVel;
        Buffer<float4> cl_comLinearForce;
        Buffer<float4> cl_comTorqueForce;
        Buffer<float16> cl_invInertialTensor;

        //Parameter structs
        Buffer<ParticleRigidBodyParams>   cl_prbp;

        void updateGPU();
        //calculate the various parameters that depend on max_num of particles
        void calculate();
        //copy the SPH parameter struct to the GPU
        void updateParams();
        float16 calculateInvInertialTensor(vector<float4>& pos, float mass);

        //Nearest Neighbors search related functions
        void call_prep(int stage);

        Buffer<float4>      cl_static_position_u;
        Buffer<float4>      cl_static_position_s;

        PRBLeapFrog leapfrog;
        PRBEuler euler;
        PRBForce force;
        PRBForceFluid forceFluid;
        MeshToParticles m2p; 
        //TODO: Need to implement Segmented scan for summing each rigid bodies particle
        //forces to find out the linear force and torque force on the center of mass.
        PRBSegmentedScan sscan;
        PRBUpdateParticles updateParticles;

		Utils u;
    };
};

#endif
