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
#include "../util.h"
#include "ShaderLibrary.h"

using namespace std;

namespace rtps
{

    ParticleEffect::ParticleEffect(ShaderLibrary* lib, GLuint width, GLuint height)
    {
        m_shaderLibrary = lib;
        this->width=width;
        this->height=height;
        //this->blending=blending;
        //this->settings=settings;
        //this->renderAsSpheres=true;
        m_writeFramebuffers = false;
        setupTimers();
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
        if(num==0)
            return;
        glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, posVBO);
        glVertexAttribPointer(0,4,GL_FLOAT,GL_FALSE,0,0);
        glBindBuffer(GL_ARRAY_BUFFER, colVBO);
        glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,0,0);

        glDrawArrays(GL_POINTS, 0, num);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glPopClientAttrib();
        //glDisableVertexAttribArray(0);
        //glDisableVertexAttribArray(1);
    }

    //----------------------------------------------------------------------
    void ParticleEffect::render(GLuint posVBO, GLuint colVBO, unsigned int num, RTPSSettings* settings, const Light* light,const Material* material, float scale, GLuint sceneTex, GLuint sceneDepthTex, GLuint framebuffer)
    {
        if(num==0)
            return;
        m_timers["render_points"]->start();
        glPushAttrib(GL_POINT_BIT|GL_ENABLE_BIT);
        bool blending=settings->GetSettingAs<bool>("blending","0");
        if(blending)
        {
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        // draws circles instead of squares
        glEnable(GL_POINT_SMOOTH);
        glPointSize(settings->GetSettingAs<float>("point_radius","0.75")*scale);

        glUseProgram(m_shaderLibrary->shaders["passThrough"].getProgram() );
        drawArrays(posVBO, colVBO, num);
        glUseProgram(0);

        if (blending)
        {
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
        }
        glPopAttrib();
        glFinish();
        m_timers["render_points"]->stop();
    }

    void ParticleEffect::writeBuffersToDisk()
    {
        m_writeFramebuffers=true;
    }

    void ParticleEffect::renderPointsAsSpheres(GLuint program, GLuint posVBO, GLuint colVBO, unsigned int num, RTPSSettings* settings, const Light* light,const Material* material, float scale)
    {
        if(num==0)
            return;
        glPushAttrib(GL_POINT_BIT|GL_ENABLE_BIT);
        glEnable(GL_POINT_SMOOTH);
        glEnable(GL_POINT_SPRITE);
        //glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
        //GLuint program = m_shaderLibrary->shaders["sphereShader"].getProgram();
	//dout<<"num = "<<num<<" point_radius = "<<settings->GetSettingAs<float>("point_radius","0.75")<<" scale = "<<scale<<endl;
        glUseProgram(program);
        glUniform1f( glGetUniformLocation(program, "pointRadius"), settings->GetSettingAs<float>("point_radius","0.75")*scale);

        drawArrays(posVBO, colVBO, num);

        glUseProgram(0);
        glPopAttrib();
    }

    void ParticleEffect::renderVector(GLuint posVBO, GLuint vecVBO,  unsigned int num, float scale)
    {
        if(num==0)
            return;
        m_timers["render_vector"]->start();
        glUseProgram(m_shaderLibrary->shaders["vectorShader"].getProgram());
        glUniform1f(glGetUniformLocation(m_shaderLibrary->shaders["vectorShader"].getProgram(), "scale"),scale);
        drawArrays(posVBO,vecVBO,num);
        glUseProgram(0);
        glFinish();
        m_timers["render_vector"]->stop();
    }
    void ParticleEffect::setupTimers()
    {
        int time_offset = 5;
        m_timers["render_vector"] = new EB::Timer("Render Vectors", time_offset);
        m_timers["render_points"] = new EB::Timer("Render Points", time_offset);
    }
    void ParticleEffect::printTimers()
    {
        cout<<"ParticleEffect Times"<<endl;
        m_timers.printAll();
        std::ostringstream oss;
        oss << "particle_effects_timer_log";
        m_timers.writeToFile(oss.str());
    }
}


