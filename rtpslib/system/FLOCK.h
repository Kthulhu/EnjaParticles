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


#ifndef RTPS_FLOCK_H_INCLUDED
#define RTPS_FLOCK_H_INCLUDED

#ifdef WIN32
#define _USE_MATH_DEFINES
#include <cmath>
#endif

#include <string>

#include "System.h"
#include "../opencl/Kernel.h"
#include "../opencl/Buffer.h"

#include "../domain/Domain.h"
#include "FLOCKSettings.h"

#include "flock/Rules.h"
#include "flock/EulerIntegration.h"

#include "./common/Hose.h"

#include "../timer_eb.h"

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

class RTPS_EXPORT FLOCK : public System
{


public:
    FLOCK(RTPSSettings* set, CL* c);
    ~FLOCK();

    void update();
    //void interact();
    void integrate();

    //wrapper around Hose.h
    int addHose(int total_n, float4 center, float4 velocity, float radius, float4 color=float4(1.0, 0.0, 0.0, 1.0f),float mass=0.0f);
    void updateHose(int index, float4 center, float4 velocity, float radius, float4 color=float4(1.0, 0.0, 0.0, 1.0f), float mass=0.0f);
    void sprayHoses();

//    virtual void render();

    GLuint getRotationVBO(){
        return rotationVBO;
    }
    int setupTimers();

    void pushParticles(vector<float4> pos, vector<float4> velo, float4 color=float4(1.0, 0.0, 0.0, 1.0), float mass = 0.0f);
    virtual void acquireGLBuffers();
    virtual void releaseGLBuffers();

private:
    FLOCKParameters flock_params;

    //keep track of hoses
    std::vector<Hose*> hoses;

    //needs to be called when particles are added
    void calculateFLOCKSettings();
    void prepareSorted();

    //This should be in OpenCL classes
    Kernel k_scopy;

    Buffer<float4>      cl_veleval_u;
    Buffer<float4>      cl_veleval_s;

    Buffer<float4>      cl_rotation_u;
    GLuint              rotationVBO;

    Buffer<int4>        cl_flockmates_s;
    Buffer<float4>      cl_separation_s;
    Buffer<float4>      cl_alignment_s;
    Buffer<float4>      cl_cohesion_s;
    Buffer<float4>      cl_goal_s;
    Buffer<float4>      cl_avoid_s;
    Buffer<float4>      cl_leaderfollowing_s;

    //Parameter structs
    Buffer<FLOCKParameters>     cl_FLOCKParameters;

    //CPU functions
    void cpuComputeRules();
    void cpuAverageRules();
	void cpuRules();
	void cpuEulerIntegration();

    void updateCPU();
    void updateGPU();

    // calculate the various parameters that depend on max_num of particles
    void calculate();

    //copy the FLOCK  parameter struct to the GPU
    void updateParams();

    //Nearest Neighbors search related functions
    void call_prep(int stage);

    Rules rules;
    EulerIntegration euler_integration;
};

}

#endif
