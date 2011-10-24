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

#include <Hash.h>
#include <BitonicSort.h>
#include <Radix.h>
#include <CellIndices.h>
#include <Permute.h> // contains CloudPermute
#include <rigidbody/PRBLeapFrog.h>
#include <rigidbody/PRBEuler.h>
#include <rigidbody/PRBForce.h>
#include <common/MeshToParticles.h>
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
        //wrapper around IV.h addRect
        //int addBox(int nn, float4 min, float4 max, bool scaled, float4 color=float4(1.0f, 0.0f, 0.0f, 1.0f));
        //wrapper around IV.h addSphere
        //void addBall(int nn, float4 center, float radius, bool scaled);

        //wrapper around Hose.h 
        //int addHose(int total_n, float4 center, float4 velocity, float radius, float4 color=float4(1.0, 0.0, 0.0, 1.0f));
        //void updateHose(int index, float4 center, float4 velocity, float radius, float4 color=float4(1.0, 0.0, 0.0, 1.0f));
        //void refillHose(int index, int refill);
        //void sprayHoses();

        virtual void render();

        //void loadTriangles(std::vector<Triangle> &triangles);

        //void testDelete();
        //int cut; //for debugging DEBUG

        EB::TimerList timers;
        int setupTimers();
        void printTimers();
        void pushParticles(vector<float4> pos, float4 velo, float4 color=float4(1.0, 0.0, 0.0, 1.0));
        void pushParticles(vector<float4> pos, vector<float4> velo, float4 color=float4(1.0, 0.0, 0.0, 1.0));

        std::vector<float4> getDeletedPos();
        std::vector<float4> getDeletedVel();

        int addBox(int nn, float4 min, float4 max, bool scaled, float4 color=float4(1.0f, 0.0f, 0.0f, 1.0f));
        //wrapper around IV.h addSphere
        void addBall(int nn, float4 center, float radius, bool scaled); 
        void addParticleShape(GLuint tex3d, float scale, float4 min, float16 world, int resolution);

    protected:
        virtual void setRenderer();
    private:
        //the particle system framework
        RTPS* ps;
        RTPSettings* settings;

        ParticleRigidBodyParams prbp;
        GridParams grid_params;
        GridParams grid_params_scaled;
        Integrator integrator;
        float spacing; //Particle rest distance in world coordinates

        std::string rigidbody_source_dir;
        int nb_var;

        std::vector<float4> deleted_pos;
        std::vector<float4> deleted_vel;

        void setupDomain();
        void prepareSorted();

        std::vector<float4> positions;
        std::vector<float4> colors;
        std::vector<float4> velocities;
        std::vector<float4> veleval;

        std::vector<float4> linearForce;
        std::vector<float4> torqueForce;
        std::vector<float4> centerOfMass;

        Buffer<float4>      cl_position_u;
        Buffer<float4>      cl_position_s;
        Buffer<float4>      cl_color_u;
        Buffer<float4>      cl_color_s;
        Buffer<float4>      cl_velocity_u;
        Buffer<float4>      cl_velocity_s;
        Buffer<float4>      cl_veleval_u;
        Buffer<float4>      cl_veleval_s;

        Buffer<float4>      cl_linear_force_s;
        Buffer<float4>      cl_linear_force_u;
        Buffer<float4>      cl_torque_force_s;
        Buffer<float4>      cl_torque_force_u;
        Buffer<float4>      cl_center_of_mass_s;
        Buffer<float4>      cl_center_of_mass_u;

        //Neighbor Search related arrays
        Buffer<unsigned int>    cl_cell_indices_start;
        Buffer<unsigned int>    cl_cell_indices_end;
        Buffer<unsigned int>    cl_sort_hashes;
        Buffer<unsigned int>    cl_sort_indices;

        //Two arrays for bitonic sort (sort not done in place)
        //should be moved to within bitonic
        Buffer<unsigned int>         cl_sort_output_hashes;
        Buffer<unsigned int>         cl_sort_output_indices;

        Bitonic<unsigned int> bitonic;
        Radix<unsigned int> radix;

        //Parameter structs
        Buffer<ParticleRigidBodyParams>   cl_prbp;
        Buffer<GridParams>  cl_GridParams;
        Buffer<GridParams>  cl_GridParamsScaled;

        Buffer<float4>      clf_debug;  //just for debugging cl files
        Buffer<int4>        cli_debug;  //just for debugging cl files


        void updateGPU();
        //calculate the various parameters that depend on max_num of particles
        void calculate();
        //copy the SPH parameter struct to the GPU
        void updateParticleRigidBodyParams();

        //Nearest Neighbors search related functions
        void call_prep(int stage);
        void hash_and_sort();
        void bitonic_sort();
        void radix_sort();
        void collision();
        void integrate();

        //Opencl kerenel classes
        Hash hash;
        CellIndices cellindices;
        Permute permute;
        //CollisionWall collision_wall;
        //CollisionTriangle collision_tri;
        PRBLeapFrog leapfrog;
        PRBEuler euler;
        PRBForce force;
        MeshToParticles m2p; 
        //TODO: Need to implement Segmented scan for summing each rigid bodies particle
        //forces to find out the linear force and torque force on the center of mass.
        //SegmentedScan sscan;

        //float Wpoly6(float4 r, float h);
        //float Wspiky(float4 r, float h);
        //float Wviscosity(float4 r, float h);

		Utils u;
    };
};

#endif
