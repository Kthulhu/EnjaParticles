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


#include "FLOCK.h"

namespace rtps{

    void FLOCK::calculate(){

        // SETTINGS
       /* float rest_distance = .05f;

        //messing with smoothing distance, making it really small to remove interaction still results in weird force values
        float smoothing_distance = 2.0f * rest_distance;

        float4 dmin = grid.getBndMin();
        float4 dmax = grid.getBndMax();
        float domain_vol = (dmax.x - dmin.x) * (dmax.y - dmin.y) * (dmax.z - dmin.z);
        float VP = 2 * .0262144 / max_num;              //Particle Volume [ m^3 ]

        float simulation_scale = pow(.5f * VP * max_num / domain_vol, 1.f/3.f) * 5.f;
        // must be less than smoothing_distance

        // SIMULATION SETTINGS
        settings->SetSetting("rest_distance", rest_distance);
        settings->SetSetting("simulation_scale", simulation_scale);

        // SPACING
        spacing =rest_distance/ simulation_scale;
        settings->SetSetting("spacing", spacing);

        // BOID SETTINGS
        settings->SetSetting("min_separation_distance", 1.f);
        settings->SetSetting("searching_radius", 1.f);
        settings->SetSetting("max_speed", 5.f);
        settings->SetSetting("angular_velocity", 0.f);

        // BOID_wEIGHTS
        settings->SetSetting("separation_weight", 1.50f);
        settings->SetSetting("alignment_weight", 0.75f);
        settings->SetSetting("cohesion_weight", 0.5f);
        settings->SetSetting("goal_weight", 0.f);
        settings->SetSetting("avoid_weight", 0.f);
        settings->SetSetting("wander_weight", 0.f);
        settings->SetSetting("leader_following_weight", 0.f);

        // BOID RULE'S SETTINGS
        settings->SetSetting("slowing_distance", 0.025f);
        settings->SetSetting("leader_index", 0);

        settings->SetSetting("max_num_particles", max_num);*/
        float smoothing_distance = settings->GetSettingAs<float>("smoothing_distance");
        float simulation_scale = settings->GetSettingAs<float>("simulation_scale");
        float rest_distance =smoothing_distance/2.0f;
        settings->SetSetting("rest_distance", rest_distance);
        //settings->SetSetting("searching_radius",smoothing_distance);
        spacing =rest_distance/ simulation_scale;
        settings->SetSetting("spacing", spacing);
        settings->SetSetting("num_particles", 0);
    }

    void FLOCK::updateParams(){

        // CL SETTINGS
        flock_params.max_num = settings->GetSettingAs<int>("max_num_particles");
        flock_params.num = settings->GetSettingAs<int>("num_particles");

        // SIMULATION SETTINGS
        flock_params.rest_distance = settings->GetSettingAs<float>("rest_distance");
        flock_params.smoothing_distance = settings->GetSettingAs<float>("smoothing_distance");
        flock_params.simulation_scale = settings->GetSettingAs<float>("simulation_scale");

        // BOID SETTINGS
        flock_params.min_dist = 0.5f * flock_params.smoothing_distance * settings->GetSettingAs<float>("min_separation_distance");
        flock_params.search_radius = 0.8f * flock_params.smoothing_distance * settings->GetSettingAs<float>("searching_radius");
        flock_params.max_speed = settings->GetSettingAs<float>("max_speed");
        flock_params.ang_vel = settings->GetSettingAs<float>("angular_velocity");

        // BOID_wEIGHTS
        flock_params.w_sep = settings->GetSettingAs<float>("separation_weight");
        flock_params.w_align = settings->GetSettingAs<float>("alignment_weight");
        flock_params.w_coh = settings->GetSettingAs<float>("cohesion_weight");
        flock_params.w_goal = settings->GetSettingAs<float>("goal_weight");
        flock_params.w_avoid = settings->GetSettingAs<float>("avoid_weight");
        flock_params.w_wander = settings->GetSettingAs<float>("wander_weight");
        flock_params.w_leadfoll = settings->GetSettingAs<float>("leader_following_weight");

        // BOID RULE'S SETTINGS
        flock_params.slowing_distance= settings->GetSettingAs<float>("slowing_distance");
        flock_params.leader_index = settings->GetSettingAs<int>("leader_index");

        // update the OpenCL buffer
        //std::vector<FLOCKParameters> vparams(0);
        //vparams.push_back(flock_params);
        cl_FLOCKParameters.copyToDevice(flock_params,0);

        settings->updated();
    }

}
