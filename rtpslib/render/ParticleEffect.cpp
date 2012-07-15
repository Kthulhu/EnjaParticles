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
#include "ShaderLibrary.h"

using namespace std;

namespace rtps
{

    ParticleEffect::ParticleEffect(ShaderLibrary* lib, GLuint width, GLuint height, GLfloat pointRadius ,bool blending)
    {
        m_shaderLibrary = lib;
        this->width=width;
        this->height=height;
        this->pointRadius=pointRadius;
        this->blending=blending;
        //this->renderAsSpheres=true;
        m_writeFramebuffers = false;
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
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, posVBO);
        glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
        glBindBuffer(GL_ARRAY_BUFFER, colVBO);
        glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,0,0);

        glDrawArrays(GL_POINTS, 0, num);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
    }

    //----------------------------------------------------------------------
    void ParticleEffect::render(GLuint posVBO, GLuint colVBO, unsigned int num, const Light* light,const Material* material, float scale )
    {

        glDepthMask(GL_TRUE);
        if(blending)
        {
            //glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        //else
        //{
           //glEnable(GL_DEPTH_TEST);
        //}

        //glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        // draws circles instead of squares
        glEnable(GL_POINT_SMOOTH);
        glPointSize(pointRadius*scale);

        //if(renderAsSpheres)
        //{
        //    renderPointsAsSpheres(posVBO,colVBO,num,light,material,scale);
        //}
        //else
        //{
            glUseProgram(m_shaderLibrary->shaders["passThrough"].getProgram() );
            drawArrays(posVBO, colVBO, num);
        //}
        glDepthMask(GL_TRUE);

        glDisable(GL_LIGHTING);

        //glDisable(GL_POINT_SMOOTH);
        if (blending)
        {
            glDisable(GL_BLEND);
        }
        //glEnable(GL_LIGHTING);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        //make sure rendering timing is accurate
        //glFinish();
    }

    void ParticleEffect::writeBuffersToDisk()
    {
        m_writeFramebuffers=true;
    }

    void ParticleEffect::renderPointsAsSpheres(GLuint posVBO, GLuint colVBO, unsigned int num, const Light* light,const Material* material, float scale)
    {

        glEnable(GL_POINT_SPRITE);
        glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
        GLuint program = m_shaderLibrary->shaders["sphereShader"].getProgram();

        glUseProgram(program);
        glUniform1f( glGetUniformLocation(program, "pointRadius"), pointRadius*scale);

        drawArrays(posVBO, colVBO, num);

        glUseProgram(0);

        glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
        glDisable(GL_POINT_SPRITE);
    }

    void ParticleEffect::renderVector(GLuint posVBO, GLuint vecVBO,  unsigned int num, float scale)
    {

        glUseProgram(m_shaderLibrary->shaders["vectorShader"].getProgram());
        glUniform1f(glGetUniformLocation(m_shaderLibrary->shaders["vectorShader"].getProgram(), "scale"),scale);
        drawArrays(posVBO,vecVBO,num);
        /*glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, posVBO);
        glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
        glBindBuffer(GL_ARRAY_BUFFER, vecVBO);
        glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,0,0);

        glDrawArrays(GL_POINTS, 0, num);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);*/
        glUseProgram(0);
    }
}


