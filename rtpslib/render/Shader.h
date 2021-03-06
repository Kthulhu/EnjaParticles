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


#ifndef RTPS_SHADER_H_INCLUDED
#define RTPS_SHADER_H_INCLUDED

#include <map>
#include <string.h>
#ifdef WIN32
//must include windows.h before gl.h on windows platform
#include <windows.h>
#endif

#if defined __APPLE__ || defined(MACOSX)
//OpenGL stuff
    //#include <GLUT/glut.h>
    #include <OpenGL/glu.h>
    #include <OpenGL/gl.h>
#else
//OpenGL stuff
    //#include <GL/glut.h>
    #include <GL/glu.h>
    #include <GL/gl.h>
#endif
#include "../rtps_common.h"

namespace rtps
{
    class RTPS_EXPORT Shader
    {
    public:
        Shader()
        {
            shaderProgram = 0;
            shaderSrc[GL_VERTEX_SHADER]="";
            #ifdef GL_TESS_CONTROL_SHADER
                shaderSrc[GL_TESS_CONTROL_SHADER]="";
                shaderSrc[GL_TESS_EVALUATION_SHADER]="";
            #endif
            #ifdef GL_GEOMETRY_SHADER
            shaderSrc[GL_GEOMETRY_SHADER]="";
            #endif
            shaderSrc[GL_FRAGMENT_SHADER]="";
        }
        ~Shader();
        void setShader(GLenum pt,const std::string& source);
        void attachGeometryParam(GLenum,GLuint);
        GLuint getProgram()
        {
            return shaderProgram;
        }
        GLuint compileProgram();
    protected:
        void compileShader(GLenum shaderType, const std::string& shaderName, GLuint program);
        GLuint shaderProgram; ///GL id for shader.
        std::map<GLenum,std::string> shaderSrc;
        std::map<GLenum,GLuint> geomParams; ///Used to setup geometry shader parameters before linking.
    };
}

#endif
