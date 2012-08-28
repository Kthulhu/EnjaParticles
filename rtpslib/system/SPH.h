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


#ifndef RTPS_SPH_H_INCLUDED
#define RTPS_SPH_H_INCLUDED

#ifdef WIN32
#define _USE_MATH_DEFINES
#include <cmath>
#endif

#include <string>


#include "System.h"
#include "SPHSettings.h"

#include "../util.h"
#include "sph/Density.h"
#include "sph/Force.h"
#include "sph/ColorField.h"
#include "sph/Collision_wall.h"
#include "sph/Collision_triangle.h"
#include "sph/RigidBodyForce.h"
#include "sph/LeapFrog.h"
#include "sph/Lifetime.h"
#include "sph/Euler.h"
#include "common/Hose.h"
#include "../rtps_common.h"

namespace rtps
{
    using namespace sph;

    class RTPS_EXPORT SPH : public System
    {
    public:
        SPH(RTPSSettings* settings, CL* c);
        ~SPH();

        void update();
        void interact();
        void integrate();
        void postProcess();
        //wrapper around Hose.h
        int addHose(int total_n, float4 center, float4 velocity, float radius, float4 color=float4(1.0, 0.0, 0.0, 1.0f), float mass = 0.0f);
        void updateHose(int index, float4 center, float4 velocity, float radius, float4 color=float4(1.0, 0.0, 0.0, 1.0f));
        void refillHose(int index, int refill);
        void sprayHoses();
        Buffer<float>& getDensityBuffer();

//        virtual void render();

        void loadTriangles(std::vector<Triangle> &triangles);

        int cut; //for debugging DEBUG

        int setupTimers();
        void printTimers();
        void pushParticles(vector<float4> pos, vector<float4> velo, float4 color=float4(1.0, 0.0, 0.0, 1.0),float mass = 0.0f);

        std::vector<float4> getDeletedPos();
        std::vector<float4> getDeletedVel();
        void acquireGLBuffers();
        void releaseGLBuffers();

        void prepareSorted();

    private:
        SPHParams sphp;

        //keep track of hoses
        std::vector<Hose*> hoses;

        //needs to be called when particles are added
        void calculateSPHSettings();

        Buffer<float4>      cl_veleval_u;
        Buffer<float4>      cl_veleval_s;

        Buffer<float>       cl_density_s;
        Buffer<float4>      cl_xsph_s;

        //Parameter structs
        Buffer<SPHParams>   cl_sphp;

        //CPU functions
        /*void cpuDensity();
        void cpuPressure();
        void cpuViscosity();
        void cpuXSPH();
        void cpuCollision_wall();
        void cpuEuler();
        void cpuLeapFrog();

        void updateCPU();*/
        void updateGPU();

        //calculate the various parameters that depend on max_num of particles
        void calculate();
        //copy the SPH parameter struct to the GPU
        void updateParams();

        //Nearest Neighbors search related functions
        //Prep prep;
        void call_prep(int stage);

        Density density;
        Force force;
        void collision();
        CollisionWall collision_wall;
        CollisionTriangle collision_tri;
        RigidBodyForce forceRB;
        ColorField colorfield;
        LeapFrog leapfrog;
        Euler euler;

        Lifetime lifetime;


        float Wpoly6(float4 r, float h);
        float Wspiky(float4 r, float h);
        float Wviscosity(float4 r, float h);

        //OpenCL helper functions, should probably be part of the OpenCL classes
        //void loadScopy();
        //void scopy(int n, cl_mem xsrc, cl_mem ydst);

        //void sset_int(int n, int val, cl_mem xdst);


		Utils u;

    };



};

#endif
