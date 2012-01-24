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


#ifndef RTPS_SYSTEM_H_INCLUDED
#define RTPS_SYSTEM_H_INCLUDED

#include "../domain/Domain.h"
#include "ForceField.h"
#include "RTPSettings.h"
//#include "../render/Render.h"
//#include "../render/SpriteRender.h"
//#include "../render/SSFRender.h"
//#include "../render/Sphere3DRender.h"

#include <RTPS.h>
#include <opencl/Kernel.h>
#include <opencl/Buffer.h>
#include <domain/Domain.h>
#include <system/common/Hash.h>
#include <system/common/BitonicSort.h>
#include <system/common/Radix.h>
#include <system/common/CellIndices.h>
#include <system/common/Permute.h> 
#include <system/common/Gravity.h> 

#include <system/common/MeshToParticles.h>
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

#include<stdio.h>
namespace rtps
{
    class RTPS;
    class RTPS_EXPORT System
    {
    public:
        virtual void update(){
            //Do update for simple system here. -ASY
        }
        virtual void interact(){
        }
        virtual void integrate(){
            
        }
        virtual void postProcess(){

        }

        System(RTPS *psfr, int num, int maxGravSources=1);
        virtual ~System();
        virtual Domain* getGrid()
        {
            return grid;
        }
        virtual int getNum()
        {
            return num;
        }
        virtual void setNum(int nn)
        {
            num = nn;
        };//should this be public
        GLuint getPosVBO()
        {
            return pos_vbo;
        }
        GLuint getColVBO()
        {
            return col_vbo;
        }
        GLuint getVelocityVBO()
        {
            return velocity_vbo;
        }
        GLuint getForceVBO()
        {
            return force_vbo;
        }

        //virtual void render();
/*
        template <typename RT>
        virtual RT GetSettingAs(std::string key, std::string defaultval = "0") 
        {
        };
        template <typename RT>
        virtual void SetSetting(std::string key, RT value) 
        {
        };
*/

        virtual int addBox(int nn, float4 min, float4 max, bool scaled, float4 color=float4(1., 0., 0., 1.),float mass = 0.0f);

        virtual void addBall(int nn, float4 center, float radius, bool scaled, float4 color=float4(1., 0., 0., 1.), float mass = 0.0f);
        virtual void addTorus(int nn, float4 center, float innerRadius, float outerRadius, float thickness, float4 color=float4(1., 0., 0., 1.), float mass = 0.0f, float innerVel=0.0f, float outerVel=0.0f);
        virtual int addHose(int total_n, float4 center, float4 velocity, float radius, float4 color=float4(1., 0., 0., 1.), float mass = 0.0f)
        {
            return 0;
        };
        virtual void updateHose(int index, float4 center, float4 velocity, float radius, float4 color=float4(1., 0., 0., 1.))
        {
        };
        virtual void refillHose(int index, int refill)
        {
        };
 
        /*
        virtual void addHose(int total_n, float4 center, float4 velocity, float radius, float spacing)
        {
        };
        */
        virtual void sprayHoses()
        {
        };

        virtual void testDelete();
        virtual void loadTriangles(std::vector<Triangle> &triangles)
        {
        };
        virtual void addForceField(ForceField ff)
        {
        };


        virtual void printTimers();

        //virtual Render* getRenderer()
        //{
        //    return renderer;
        //}
        
        void pushParticles(vector<float4> pos, float4 velo, float4 color=float4(1.0, 0.0, 0.0, 1.0),float mass = 0.0f);
        virtual void pushParticles(vector<float4> pos, vector<float4> velo, float4 color=float4(1.0, 0.0, 0.0, 1.0),float mass = 0.0f){return;}
        virtual void addParticleShape(GLuint tex3d,float scale,float4 min,float16 world,int voxelResolution,float4 velo=float4(0.0, 0.0, 0.0, 0.0), float4 color=float4(1.0, 0.0, 0.0, 1.0), float mass = 0.0f);
        virtual void setupDomain(float cell_size, float sim_scale);
        virtual void prepareSorted();
        virtual int setupTimers();
        void addPointSource(float4& pointSource, float massSource);
        void setAlpha(float newAlpha)
        {
            alpha = newAlpha;
        }
        void addInteractionSystem(System* interact)
        {
            interactionSystem.push_back(interact);
        }
        Buffer<float4>& getVelocityBuffer() {return cl_velocity_s;}
        Buffer<float4>& getColorBuffer() {return cl_color_s;}
        Buffer<float4>& getPositionBuffer() {return cl_position_s;}
        Buffer<float>& getMassBuffer() {return cl_mass_s;}
        Buffer<float4>& getVelocityBufferUnsorted() {return cl_velocity_u;}
        Buffer<float4>& getColorBufferUnsorted() {return cl_color_u;}
        Buffer<float4>& getPositionBufferUnsorted() {return cl_position_u;}
        Buffer<float>& getMassBufferUnsorted() {return cl_mass_u;}
        Buffer<float>& getSpringCoefBuffer() {return cl_spring_coef_s;}
        Buffer<float>& getDampeningCoefBuffer() {return cl_dampening_coef_s;}
        Buffer<unsigned int>& getCellStartBuffer() {return cl_cell_indices_start;}
        Buffer<unsigned int>& getCellEndBuffer() {return cl_cell_indices_end;}
        float getSpacing(){return spacing;}
        RTPSettings* getSettings() {return settings;}
        virtual void acquireGLBuffers();
        virtual void releaseGLBuffers();
    protected:
                //the particle system framework
        RTPS* ps;
        RTPSettings* settings;

        EB::TimerList timers;

        std::vector<float4> deleted_pos;
        std::vector<float4> deleted_vel;

        //SPHSettings* sphsettings;
        GridParams grid_params;
        GridParams grid_params_scaled;

        Buffer<float4>      cl_position_u;
        Buffer<float4>      cl_position_s;
        Buffer<float4>      cl_color_u;
        Buffer<float4>      cl_color_s;
        Buffer<float4>      cl_velocity_u;
        Buffer<float4>      cl_velocity_s;
        Buffer<float4>      cl_force_s;
        Buffer<float4>      cl_active_cells;
        Buffer<float>      cl_mass_u;
        Buffer<float>      cl_mass_s;
        Buffer<unsigned int>      cl_objectIndex_u;
        Buffer<unsigned int>      cl_objectIndex_s;

        Buffer<unsigned int>    cl_cell_indices_start;
        Buffer<unsigned int>    cl_cell_indices_end;
        //Two arrays for bitonic sort (sort not done in place)
        //should be moved to within bitonic
        Buffer<unsigned int>    cl_sort_output_hashes;
        Buffer<unsigned int>    cl_sort_output_indices;
        Buffer<unsigned int>    cl_sort_hashes;
        Buffer<unsigned int>    cl_sort_indices;
        Buffer<GridParams>  cl_GridParams;
        Buffer<GridParams>  cl_GridParamsScaled;

        Buffer<float>      cl_spring_coef_u;
        Buffer<float>      cl_spring_coef_s;
        Buffer<float>      cl_dampening_coef_u;
        Buffer<float>      cl_dampening_coef_s;

        Buffer<float4>      clf_debug;  //just for debugging cl files
        Buffer<int4>        cli_debug;  //just for debugging cl files
        Bitonic<unsigned int> bitonic;
        Radix<unsigned int> radix;

        //Gravity
        Buffer<float4> cl_pointSources;
        Buffer<float> cl_massSources;
        float alpha;
        int numGravSources;
        int maxGravSources;
        
        float spacing; //Particle rest distance in world coordinates
        //number of particles
        int num;
        //maximum number of particles (for array allocation)
        int max_num;
        //Used for debug views of a specific particle;
        unsigned int activeParticle;

        GLuint pos_vbo;
        GLuint col_vbo;
        GLuint velocity_vbo;
        GLuint force_vbo;
        GLuint active_cells_vbo;

        Domain* grid;

        //Render* renderer;

        std::string resource_path;
        std::string common_source_dir;

        Hash hash;
        CellIndices cellindices;
        Permute permute;
        Gravity gravity;
        MeshToParticles m2p; 
        vector<System*> interactionSystem;

        void hash_and_sort();
        void bitonic_sort();
        void radix_sort();
        virtual void updateParams(){};
        //virtual void setRenderer();
    };

}

#endif
