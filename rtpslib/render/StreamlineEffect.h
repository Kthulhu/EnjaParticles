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


#ifndef RTPS_STREAMLINE_EFFECT_H_INCLUDED
#define RTPS_STREAMLINE_EFFECT_H_INCLUDED

#include <map>
#include <vector>
#include <string.h>

#ifdef WIN32
//must include windows.h before gl.h on windows platform
#include <windows.h>
#endif

#if defined __APPLE__ || defined(MACOSX)
//OpenGL stuff
    #include <OpenGL/glu.h>
    #include <OpenGL/gl.h>
#else
//OpenGL stuff
    #include <GL/glu.h>
    #include <GL/gl.h>
#endif

#include "../structs.h"
#include "../timer_eb.h"
#include "ParticleEffect.h"

namespace rtps
{
    class StreamlineEffect : public ParticleEffect
    {
    public:
        StreamlineEffect(RenderSettings rs, ShaderLibrary& lib, unsigned int maxLength, unsigned int num);
        ~StreamlineEffect();

        virtual void render(GLuint posVBO, GLuint colVBO, unsigned int num);
        void writeBuffersToDisk();

    protected:
        unsigned int m_maxSLLength;
        unsigned int m_curSLLength;
        unsigned int m_numSL;
        GLuint m_streamLineCP;
        GLuint m_streamLineColor;
    };  
}

#endif
