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


/*#include <GL/glew.h>
#if defined __APPLE__ || defined(MACOSX)
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif*/
#include <GL/glew.h>

#include "SSEffect.h"
#include "ShaderLibrary.h"
#include "Shader.h"

using namespace std;
namespace rtps
{
    SSEffect::SSEffect(ShaderLibrary* lib, SmoothingFilter filter = SmoothingFilter::GUASSIAN_BLUR, GLuint width, GLuint height, GLfloat pointRadius,bool blending):
        ParticleEffect(lib,width,height,pointRadius,blending)
    {
        cout<<"Shaderlib size = "<<m_shaderLibrary.shaders.size()<<endl;
        m_fbos.resize(1);
        glGenFramebuffersEXT(1,&m_fbos[0]);
        createFramebufferTextures();
        smoothing=filter;
        currentDepthBuffer="depth";
        glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT,m_fbos[0]);
        glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["thickness"],0);
        glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT1_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["depthColor"],0);
        glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT2_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["normalColor"],0);
        glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT3_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["lightColor"],0);
        glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT4_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["Color"],0);
        glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT5_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["depthColorSmooth"],0);
        glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["depth"],0);
        glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT,0);
    }
    void SSEffect::smoothDepth()
    {
        GLuint smoothingProgram;
        switch(smoothing)
        {
            case NO_SMOOTHING:
                currentDepthBuffer="depth";
                return;
            case SEPERABLE_GAUSS_BLUR:
                smoothingProgram= m_shaderLibrary->shaders["gaussBlurXShader"].getProgram();
                glUseProgram(smoothingProgram);
                glUniform1i( glGetUniformLocation(smoothingProgram, "depthTex"),0);
                glUniform1i( glGetUniformLocation(smoothingProgram, "width"),width);
                RenderUtils::fullscreenQuad();

                glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer]);
                currentDepthBuffer="depth";
                glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer],0);
                glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
                smoothingProgram= m_shaderLibrary->shaders["gaussBlurYShader"].getProgram();
                glUseProgram(smoothingProgram);
                glUniform1i( glGetUniformLocation(smoothingProgram, "depthTex"),0);
                glUniform1i( glGetUniformLocation(smoothingProgram, "height"),height);
                currentDepthBuffer="depth2";
                break;
            case BILATERAL_GAUSSIAN_BLUR:
                glUseProgram(m_shaderLibrary->shaders["bilateralGaussianBlurShader"].getProgram());
                glUniform1i(glGetUniformLocation(m_shaderLibrary->shaders["bilateralGaussianBlurShader"].getProgram(),"depthTex"),0);
                glUniform1f( glGetUniformLocation(m_shaderLibrary->shaders["bilateralGaussianBlurShader"].getProgram(), "del_x"),1.0f/(width));
                glUniform1f( glGetUniformLocation(m_shaderLibrary->shaders["bilateralGaussianBlurShader"].getProgram(), "del_y"),1.0f/(height));
                currentDepthBuffer="depth";
                break;
            case CURVATURE_FLOW:
            glUseProgram(m_shaderLibrary.shaders["curvatureFlowShader"].getProgram());
            glUniform1i(glGetUniformLocation(m_shaderLibrary->shaders["curvatureFlowShader"].getProgram(),"depthTex"),0);
            glUniform1i(glGetUniformLocation(m_shaderLibrary->shaders["curvatureFlowShader"].getProgram(),"width"),width);
            glUniform1i(glGetUniformLocation(m_shaderLibrary->shaders["curvatureFlowShader"].getProgram(),"height"),height);
            for(unsigned int i = 0; i<numberOfCurvatureIterations; i++)
            {
                RenderUtils::fullscreenQuad();
                glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer]);
                if(i%2)
                    currentDepthBuffer="depth";
                else
                    currentDepthBuffer="depth2";
                glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer],0);
                glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            }
            return;
            default:
                break;
        }
        RenderUtils::fullscreenQuad();
    }

    void SSEffect::render(GLuint posVBO, GLuint colVBO, unsigned int num, const Light* light,const Material* material, float scale)
    {

        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

        //perserve original buffer
        GLint buffer;
        glGetIntegerv(GL_DRAW_BUFFER,&buffer);

        glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT,m_fbos[0]);

        //Should probably conditionally create the thickness buffer as well.
        //Render Thickness buffer.
        if(thickness)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            //glDrawBuffers(2,buffers);
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
            glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
            glClearColor(0.0f,0.0f,0.0f,0.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            renderPointsAsSpheres(posVBO, colVBO, num, light,material,scale);
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }

        //Render Color and depth buffer of spheres.
        glDrawBuffer(GL_COLOR_ATTACHMENT4_EXT);
        glClearColor(0.0f,0.0f,0.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderPointsAsSpheres(posVBO, colVBO, num, light,material,scale);
        //smoothDepth();


        //Smooth the depth texture to emulate a surface.
        //glDrawBuffer(GL_COLOR_ATTACHMENT1);
        //glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
        glDrawBuffer(GL_COLOR_ATTACHMENT5_EXT);
        glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer]);
        currentDepthBuffer="depth2";
        glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer],0);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);


        smoothDepth();
        glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer],0);
        //If no shader was used to smooth then we need the original depth texture

        glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer]);
        //glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,800,600);


        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        if (blending)
        {
            //glDepthMask(GL_FALSE);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        //glBindFramebuffer(GL_DRAW_FRAMEBUFFER,fbos[0]);
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs["Color"]);
        //Render the normals for the new "surface".
        glDrawBuffer(GL_COLOR_ATTACHMENT2_EXT);
        glClearColor(0.0f,0.0f,0.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        GLuint normalProgram = m_shaderLibrary->shaders["depth2NormalShader"].getProgram();
        glUseProgram(normalProgram);
        glUniform1i( glGetUniformLocation(normalProgram, "depthTex"),0);
        //glUniform1i( glGetUniformLocation(normalProgram, "colorTex"),1);
        glUniform1f( glGetUniformLocation(normalProgram, "del_x"),1.0/((float)m_settings.windowWidth));
        glUniform1f( glGetUniformLocation(normalProgram, "del_y"),1.0/((float)m_settings.windowHeight));
        glUniform3fv(glGetUniformLocation(normalProgram,"material.diffuse"),1,&mesh->material.diffuse.x);
        glUniform3fv(glGetUniformLocation(normalProgram,"material.specular"),1,&mesh->material.specular.x);
        glUniform3fv(glGetUniformLocation(normalProgram,"material.ambient"),1,&mesh->material.ambient.x);
        glUniform1fv(glGetUniformLocation(normalProgram,"material.shininess"),1,&mesh->material.shininess);
        glUniform1fv(glGetUniformLocation(normalProgram,"material.opacity"),1,&mesh->material.opacity);
        glUniform3fv(glGetUniformLocation(normalProgram,"light->diffuse"),1,&light->diffuse.x);
        glUniform3fv(glGetUniformLocation(normalProgram,"light->specular"),1,&light->specular.x);
        glUniform3fv(glGetUniformLocation(normalProgram,"light->ambient"),1,&light->ambient.x);
        glUniform3fv(glGetUniformLocation(normalProgram,"light->pos"),1,&light->pos.x);
        RenderUtils::fullscreenQuad(width, height);

        //TODO: should add another shader for performing compositing

        /*
        glUseProgram(0);
        glBindTexture(GL_TEXTURE_2D,0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,0);
        */

        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);

        glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT,0);
        glDrawBuffer(buffer);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs["normalColor"]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer]);
        GLuint copyProgram = m_shaderLibrary->shaders["copyShader"].getProgram();
        glUseProgram(copyProgram);
        glUniform1i( glGetUniformLocation(copyProgram, "normalTex"),0);
        glUniform1i( glGetUniformLocation(copyProgram, "depthTex"),1);
        RenderUtils::fullscreenQuad();



        glUseProgram(0);
        glBindTexture(GL_TEXTURE_2D,0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,0);
        //printf("done rendering, clean up\n");


        //glDisable(GL_POINT_SMOOTH);
        if (blending)
        {
            glDisable(GL_BLEND);
        }

        glPopClientAttrib();
        glPopAttrib();

        //glEnable(GL_LIGHTING);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        //make sure rendering timing is accurate
        glFinish();

        //printf("done rendering\n");
        if (m_writeFramebuffers)
        {
            writeFramebufferTextures();
            m_writeFramebuffers = false;
        }
    }

    void SSEffect::deleteFramebufferTextures()
    {
        glDeleteTextures(1,&m_glFramebufferTexs["depth"]);
        glDeleteTextures(1,&m_glFramebufferTexs["depth2"]);
        glDeleteTextures(1,&m_glFramebufferTexs["thickness"]);
        glDeleteTextures(1,&m_glFramebufferTexs["depthColor"]);
        glDeleteTextures(1,&m_glFramebufferTexs["depthColorSmooth"]);
        glDeleteTextures(1,&m_glFramebufferTexs["normalColor"]);
        glDeleteTextures(1,&m_glFramebufferTexs["lightColor"]);
        glDeleteTextures(1,&m_glFramebufferTexs["Color"]);
    }

    void SSEffect::createFramebufferTextures()
    {
        glGenTextures(1, &m_glFramebufferTexs["depth"]);
        glBindTexture(GL_TEXTURE_2D, m_glFramebufferTexs["depth"]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT32,m_settings.windowWidth,m_settings.windowHeight,0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);
        glGenTextures(1, &m_glFramebufferTexs["depth2"]);
        glBindTexture(GL_TEXTURE_2D, m_glFramebufferTexs["depth2"]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT32,m_settings.windowWidth,m_settings.windowHeight,0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);
        glGenTextures(1,&m_glFramebufferTexs["thickness"]);
        glBindTexture(GL_TEXTURE_2D, m_glFramebufferTexs["thickness"]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,m_settings.windowWidth,m_settings.windowHeight,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
        //glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA32F,m_settings.windowWidth,m_settings.windowHeight,0,GL_RGBA,GL_FLOAT,NULL);
        glGenTextures(1,&m_glFramebufferTexs["depthColor"]);
        glBindTexture(GL_TEXTURE_2D, m_glFramebufferTexs["depthColor"]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,m_settings.windowWidth,m_settings.windowHeight,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
        glGenTextures(1,&m_glFramebufferTexs["normalColor"]);
        glBindTexture(GL_TEXTURE_2D, m_glFramebufferTexs["normalColor"]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,m_settings.windowWidth,m_settings.windowHeight,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
        //glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA32F,m_settings.windowWidth,m_settings.windowHeight,0,GL_RGBA,GL_FLOAT,NULL);
        glGenTextures(1,&m_glFramebufferTexs["lightColor"]);
        glBindTexture(GL_TEXTURE_2D, m_glFramebufferTexs["lightColor"]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,m_settings.windowWidth,m_settings.windowHeight,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
        glGenTextures(1,&m_glFramebufferTexs["Color"]);
        glBindTexture(GL_TEXTURE_2D, m_glFramebufferTexs["Color"]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,m_settings.windowWidth,m_settings.windowHeight,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
        glGenTextures(1,&m_glFramebufferTexs["depthColorSmooth"]);
        glBindTexture(GL_TEXTURE_2D, m_glFramebufferTexs["depthColorSmooth"]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,m_settings.windowWidth,m_settings.windowHeight,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
        //glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA32F,m_settings.windowWidth,m_settings.windowHeight,0,GL_RGBA,GL_FLOAT,NULL);

    }

    void SSEffect::setWindowDimensions(GLuint width, GLuint height)
    {
        deleteFramebufferTextures();
        ParticleEffect::setWindowDimensions(width,height);
        createFramebufferTextures();
        glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT,m_fbos[0]);
        glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["thickness"],0);
        glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT1_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["depthColor"],0);
        glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT2_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["normalColor"],0);
        glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT3_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["lightColor"],0);
        glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT4_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["Color"],0);
        glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT5_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["depthColorSmooth"],0);
        glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["depth"],0);
        glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT,0);
    }

};
