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


#include <GL/glew.h>
#include <iostream>

#include "Shader.h"
#include "util.h"

using namespace std;

namespace rtps
{

    Shader::~Shader()
    {
        if(shaderProgram)
            glDeleteProgram(shaderProgram);
    }

    void Shader::setShader(GLenum shaderType, const string& shadeSrc)
    {
        shaderSrc[shaderType] = shadeSrc;
    }

    void Shader::attachGeometryParam(GLenum paramType, GLuint paramValue)
    {
        geomParams[paramType]=paramValue;
    }

    GLuint Shader::compileProgram()
    {
        GLuint program = glCreateProgram();

        compileShader(GL_VERTEX_SHADER, "Vertex Shader",  program);
        compileShader(GL_FRAGMENT_SHADER, "Fragment Shader",  program);
		#ifdef GL_GEOMETRY_SHADER
        if (shaderSrc[GL_GEOMETRY_SHADER].length())
        {
            compileShader(GL_GEOMETRY_SHADER, "Geometry Shader",program);
            for (map<GLenum,GLuint>::iterator i =geomParams.begin();i!=geomParams.end(); i++)
            {
                glProgramParameteriEXT(program,i->first,i->second);
            }
        }
		#endif

        #ifdef GL_TESS_CONTROL_SHADER
        if (shaderSrc[GL_TESS_CONTROL_SHADER].length())
        {
            compileShader(GL_TESS_CONTROL_SHADER, "Tesselation Control Shader",program);
        }

        if (shaderSrc[GL_TESS_EVALUATION_SHADER].length())
        {
            compileShader(GL_TESS_EVALUATION_SHADER, "Tesselation Evaluation Shader",program);
        }
        #endif

        glLinkProgram(program);

        // check if program linked
        GLint success = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            char temp[256];
            glGetProgramInfoLog(program, 256, 0, temp);
            cerr<<"Failed to link program:\n"<<temp<<endl;
            glDeleteProgram(program);
            program = 0;
        }
        shaderProgram = program;

        return program;
    }

    void Shader::compileShader(GLenum shaderType, const string& shaderName, GLuint program)
    {
        GLuint shader = glCreateShader(shaderType);
        GLint success;
        GLint len=0;
        const char* src = shaderSrc[shaderType].c_str();
        glShaderSource(shader, 1, (const GLchar**)&src, 0);
        glCompileShader(shader);
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        if (len>0 && !success)
        {
            char* log = new char[len+1];
            glGetShaderInfoLog(shader, len+1, 0, log);
            cout<<shaderName<<"\n"<<log<<endl;
			delete[] log;
        }
        glAttachShader(program, shader);
        glDeleteShader(shader);
    }
}


