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


#ifndef RTPS_RENDERUTILS_H_INCLUDED
#define RTPS_RENDERUTILS_H_INCLUDED

#include <string>
#include <map>

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

namespace rtps
{
    class RenderUtils
    {
    public:
        static void orthoProjection();
        static void perspectiveProjection();
        static void fullscreenQuad();
        static void renderBox(float4 min, float4 max, float4 color=float4(0.0f,1.0f,0.0f,1.0f));
        static void renderQuad(float4 min, float4 max, GLuint tex = 0); //put in render utils
        static void writeTextures(const std::map<std::string,GLuint>& texs);
        static int writeTexture(GLuint tex, const std::string& filename, bool depth);
        static void write3DTextureToDisc(GLuint tex,int voxelResolution, const char* filename)
        static GLuint loadTexture(const std::string& texFile, const std::string& texName);
        static void convertDepthToRGB(const GLfloat* depth, GLuint size, GLubyte* rgb);
    };  
}
#endif
