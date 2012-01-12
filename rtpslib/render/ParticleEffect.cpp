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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <GL/glew.h>

#include "ParticleEffect.h"
#include "Shader.h"
#include "util.h"
#include "stb_image.h" 
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "ShaderLibrary.h" 

using namespace std;

namespace rtps
{

    ParticleEffect::ParticleEffect(RenderSettings rs)
    {
        m_settings=rs;
        m_writeFramebuffers = false;
        setupTimers();
        //Fixme:: Should probably handle shader loading more elegantly. -ASY 12/14/2011
        if(m_shaderLibrary.shaders.size()==0)
        {
            m_shaderLibrary.initializeShaders(GLSL_BIN_DIR);
        }
        
    }

    

    //----------------------------------------------------------------------
    ParticleEffect::~ParticleEffect()
    {
        for (map<string,GLuint>::iterator i = m_glFramebufferTexs.begin();i!=m_glFramebufferTexs.end();i++)
        {
            glDeleteTextures(1,&(i->second));
        }
        if (m_fbos.size())
        {
            glDeleteFramebuffersEXT(m_fbos.size(),&m_fbos[0]);
        }
        //}
    }

    //----------------------------------------------------------------------
    void ParticleEffect::drawArrays(GLuint posVBO, GLuint colVBO, unsigned int num)
    {
        glBindBuffer(GL_ARRAY_BUFFER, colVBO);
        glColorPointer(4, GL_FLOAT, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, posVBO);
        glVertexPointer(4, GL_FLOAT, 0, 0);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);

        //Need to disable these for blender
        glDisableClientState(GL_NORMAL_ARRAY);
        glDrawArrays(GL_POINTS, 0, num);

        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }

    //----------------------------------------------------------------------
    void ParticleEffect::render(GLuint posVBO, GLuint colVBO, unsigned int num)
    {

        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

        glDepthMask(GL_TRUE);
        if (m_settings.blending)
        {
            //glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        else
        {
            //glEnable(GL_DEPTH_TEST);
        }

        //glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        // draws circles instead of squares
        glEnable(GL_POINT_SMOOTH); 
        //TODO make the point size a setting
        glPointSize(m_settings.particleRadius);

        drawArrays(posVBO, colVBO, num);
        glDepthMask(GL_TRUE);

        glDisable(GL_LIGHTING);

        glPopClientAttrib();
        glPopAttrib();
        //glDisable(GL_POINT_SMOOTH);
        if (m_settings.blending)
        {
            glDisable(GL_BLEND);
        }
        //glEnable(GL_LIGHTING);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        //make sure rendering timing is accurate
        glFinish();
    }
    
    void ParticleEffect::writeBuffersToDisk()
    {
        m_writeFramebuffers=true;
    }

    void ParticleEffect::renderPointsAsSpheres(GLuint posVBO, GLuint colVBO, unsigned int num)
    {

        glEnable(GL_POINT_SPRITE);
        glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
        GLuint program = m_shaderLibrary.shaders["vectorShader"].getProgram();

        glUseProgram(program);
        //float particle_radius = 0.125f * 0.5f;
        glUniform1f( glGetUniformLocation(program, "pointScale"), ((float)m_settings.windowWidth) / tanf(65. * (0.5f * 3.1415926535f/180.0f)));

        glUniform1f( glGetUniformLocation(program, "pointRadius"), m_settings.particleRadius);
        glUniform1f( glGetUniformLocation(program, "near"), m_settings.near );
        glUniform1f( glGetUniformLocation(program, "far"), m_settings.far);

        //glColor3f(1., 1., 1.);

        drawArrays(posVBO, colVBO, num);

        glUseProgram(0);

        glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
        glDisable(GL_POINT_SPRITE);
    }

    void ParticleEffect::renderVector(GLuint vecVBO, GLuint posVBO, unsigned int num, float scale)
    {
        glBindBuffer(GL_ARRAY_BUFFER, vecVBO);
        glColorPointer(4, GL_FLOAT, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, posVBO);
        glVertexPointer(4, GL_FLOAT, 0, 0);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);

        glUseProgram(m_shaderLibrary.shaders["vectorShader"].getProgram());
        glUniform1f(glGetUniformLocation(m_shaderLibrary.shaders["vectorShader"].getProgram(), "scale"),scale);
        //Need to disable these for blender
        glDisableClientState(GL_NORMAL_ARRAY);
        glDrawArrays(GL_POINTS, 0, num);
        glUseProgram(0);

        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }
}


