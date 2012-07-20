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
    SSEffect::SSEffect(ShaderLibrary* lib, GLuint width, GLuint height)://, GLfloat pointRadius,bool blending):
        ParticleEffect(lib,width,height)//,pointRadius,blending)
    {
        //cout<<"Shaderlib size = "<<m_shaderLibrary.shaders.size()<<endl;
        m_fbos.resize(1);
        glGenFramebuffersEXT(1,&m_fbos[0]);
        createFramebufferTextures();
        currentDepthBuffer="depth";
        glEnable(GL_TEXTURE_2D);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,m_fbos[0]);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["thickness1"],0);

        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["depth"],0);

        dout<<"fbo[0] = "<<m_fbos[0]<<" status complete? "<<((glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE)?"yes":"no")<<" "<<glCheckFramebufferStatus(GL_FRAMEBUFFER)<<endl;
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);

        glDisable(GL_TEXTURE_2D);
    }
    void SSEffect::smoothDepth( RTPSSettings* settings)
    {
        GLuint smoothingProgram;
        switch(settings->GetSettingAs<int>("filter_type","0"))
        {
            case NO_SMOOTHING:
                return;
            case SEPERABLE_GAUSSIAN_BLUR:
                smoothingProgram= m_shaderLibrary->shaders["gaussianBlurXShader"].getProgram();
                glUseProgram(smoothingProgram);
                glUniform1i( glGetUniformLocation(smoothingProgram, "depthTex"),0);
                glUniform1f( glGetUniformLocation(smoothingProgram, "del_x"),1.0f/(float)width);
                glUniform1f( glGetUniformLocation(smoothingProgram, "falloff"),settings->GetSettingAs<float>("blur_falloff","1.0"));
                glUniform1f( glGetUniformLocation(smoothingProgram, "sig"),settings->GetSettingAs<float>("blur_radius","8.0"));
                RenderUtils::fullscreenQuad();


                glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["depth"],0);
                glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs["depth2"]);

                glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
                smoothingProgram= m_shaderLibrary->shaders["gaussianBlurYShader"].getProgram();
                glUseProgram(smoothingProgram);
                glUniform1i( glGetUniformLocation(smoothingProgram, "depthTex"),0);
                glUniform1f( glGetUniformLocation(smoothingProgram, "del_y"),1.0f/(float)height);
                glUniform1f( glGetUniformLocation(smoothingProgram, "falloff"),settings->GetSettingAs<float>("blur_falloff","1.0"));
                glUniform1f( glGetUniformLocation(smoothingProgram, "sig"),settings->GetSettingAs<float>("blur_radius","8.0"));
                currentDepthBuffer="depth2";
                break;
            case GAUSSIAN_BLUR:
                smoothingProgram= m_shaderLibrary->shaders["gaussianBlurShader"].getProgram();
                glUseProgram(smoothingProgram);
                glUniform1i(glGetUniformLocation(smoothingProgram,"depthTex"),0);
                glUniform1f( glGetUniformLocation(smoothingProgram, "del_x"),1.0/((float)width));
                glUniform1f( glGetUniformLocation(smoothingProgram, "del_y"),1.0/((float)height));
                glUniform1f( glGetUniformLocation(smoothingProgram, "falloff"),settings->GetSettingAs<float>("blur_falloff","1.0"));
                glUniform1f( glGetUniformLocation(smoothingProgram, "sig"),settings->GetSettingAs<float>("blur_radius","8.0"));
                currentDepthBuffer="depth";
                break;
            case BILATERAL_GAUSSIAN_BLUR:
            {
                smoothingProgram= m_shaderLibrary->shaders["bilateralGaussianBlurShader"].getProgram();
                float xdir[] = {1.0f/height,0.0f};
                float ydir[] = {0.0f,1.0f/width};
                glUseProgram(smoothingProgram);
                glUniform1i( glGetUniformLocation(smoothingProgram, "depthTex"),0);
                glUniform2fv( glGetUniformLocation(smoothingProgram, "blurDir"),1,xdir);
                glUniform1f( glGetUniformLocation(smoothingProgram, "sig_range"),settings->GetSettingAs<float>("bilateral_range","0.01"));
                glUniform1f( glGetUniformLocation(smoothingProgram, "sig"),settings->GetSettingAs<float>("blur_radius","8.0"));
                RenderUtils::fullscreenQuad();
                glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["depth"],0);

                //glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer],0);
                glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs["depth2"]);
                glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

                glUniform2fv( glGetUniformLocation(smoothingProgram, "blurDir"),1,ydir);
                //glUniform1f( glGetUniformLocation(smoothingProgram, "sig_range"),bilateralRange);
                //glUniform1f( glGetUniformLocation(glsl_program[BILATERAL_GAUSSIAN_SHADER], "sig"),filterRadius);
                currentDepthBuffer="depth2";
                break;
            }
            case CURVATURE_FLOW:
            {
                int numberIterations=settings->GetSettingAs<float>("curvature_flow_iterations","20");
                smoothingProgram= m_shaderLibrary->shaders["curvatureFlowShader"].getProgram();
                glUniform1i(glGetUniformLocation(smoothingProgram,"depthTex"),0);
                //glUniform1f(glGetUniformLocation(glsl_program[CURVATURE_FLOW_SHADER],"width"),(float)window_width);
                //glUniform1f(glGetUniformLocation(glsl_program[CURVATURE_FLOW_SHADER],"height"),(float)window_height);
                glUniform1f( glGetUniformLocation(smoothingProgram, "del_x"),1.0/((float)width));
                glUniform1f( glGetUniformLocation(smoothingProgram, "del_y"),1.0/((float)height));
                glUniform1f( glGetUniformLocation(smoothingProgram, "h_x"),1.0/((float)width-1));
                glUniform1f( glGetUniformLocation(smoothingProgram, "h_y"),1.0/((float)height-1));
                //glUniform1f( glGetUniformLocation(glsl_program[CURVATURE_FLOW_SHADER], "focal_x"),focal_x);
                //glUniform1f( glGetUniformLocation(glsl_program[CURVATURE_FLOW_SHADER], "focal_y"),focal_y);
                glUniform1f( glGetUniformLocation(smoothingProgram, "dt"),1.0f/numberIterations);
                //glUniform1f( glGetUniformLocation(smoothingProgram, "distance_threshold"), falloff);
                glUseProgram(smoothingProgram);

                for(unsigned int i = 0; i<numberIterations; i++)
                {
                    RenderUtils::fullscreenQuad();
                    glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer]);
                    if(i%2)
                        currentDepthBuffer="depth";
                    else
                        currentDepthBuffer="depth2";
                    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer],0);

                    //glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer],0);
                    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
                }
                glUseProgram(0);
                return;
            }
            default:
                break;
        }
        RenderUtils::fullscreenQuad();
        glUseProgram(0);
    }

    void SSEffect::render(GLuint posVBO, GLuint colVBO, unsigned int num, RTPSSettings* settings, const Light* light,const Material* material, float scale, GLuint sceneTex, GLuint framebuffer)
    {
        if(num==0)
            return;
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
        currentDepthBuffer="depth";

        bool blending = settings->GetSettingAs<bool>("blending","1");
        bool thickness = settings->GetSettingAs<bool>("thickness","1");
        //perserve original buffer
        GLint buffer;
        glGetIntegerv(GL_DRAW_BUFFER,&buffer);
        glClearColor(0.0f,0.0f,0.0f,0.0f);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,m_fbos[0]);
        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);

        //Should probably conditionally create the thickness buffer as well.
        //Render Thickness buffer.
        if(thickness)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["thickness1"],0);
            //glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            renderPointsAsSpheres( m_shaderLibrary->shaders["sphereThicknessShader"].getProgram(),posVBO, colVBO, num,settings, light,material,scale);
            //renderPointsAsSpheres( m_shaderLibrary->shaders["sphereShader"].getProgram(),posVBO, colVBO, num,settings, light,material,scale);

            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["thickness2"],0);
            GLuint program= m_shaderLibrary->shaders["fixedWidthGaussianShader"].getProgram();
            glEnable(GL_TEXTURE_2D);
            float xdir[] = {1.0f/height,0.0f};
            float ydir[] = {0.0f,1.0f/width};
            glUseProgram(program);
            glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs["thickness1"]);
            glUniform1i( glGetUniformLocation(program, "imgTex"),0);
            glUniform2fv( glGetUniformLocation(program, "dTex"),1,xdir);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            RenderUtils::fullscreenQuad();
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["thickness1"],0);
            glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs["thickness2"]);
            glUniform2fv( glGetUniformLocation(program, "dTex"),1,ydir);
            glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            RenderUtils::fullscreenQuad();

            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
            glDisable(GL_TEXTURE_2D);
        }

        //Render Color and depth buffer of spheres.
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["Color"],0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderPointsAsSpheres( m_shaderLibrary->shaders["sphereShader"].getProgram(),posVBO, colVBO, num,settings, light,material,scale);

        glEnable(GL_TEXTURE_2D);
        //Smooth the depth texture to emulate a surface.
        //glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["Color"],0);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["depth2"],0);
        glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs["depth"]);
        currentDepthBuffer="depth2";
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        smoothDepth(settings);


        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer],0);
        //Switch to the buffer that was written to in the smoothing step
        if(currentDepthBuffer=="depth2")
            currentDepthBuffer="depth";
        else
            currentDepthBuffer="depth2";

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer]);

        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        //if (blending)
        //{
            //glDepthMask(GL_FALSE);
        //    glEnable(GL_BLEND);
        //    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        //}
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["normalColor"],0);

        //Now we use the depth to normal shader which converts screenspace depth into
        //world coordinates and then computes lighting.
        glClear(GL_COLOR_BUFFER_BIT);
        GLuint normalProgram = m_shaderLibrary->shaders["depth2NormalShader"].getProgram();
        glUseProgram(normalProgram);
        glUniform1i( glGetUniformLocation(normalProgram, "depthTex"),0);
        glUniform1f( glGetUniformLocation(normalProgram, "del_x"),1.0/((float)width));
        glUniform1f( glGetUniformLocation(normalProgram, "del_y"),1.0/((float)height));
        RenderUtils::fullscreenQuad();


        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["Color"],0);
        GLuint compositeProgram = m_shaderLibrary->shaders["compositeFluidShader"].getProgram();
        glUseProgram(compositeProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs["normalColor"]);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs["thickness1"]);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D,sceneTex);
        glUniform1i( glGetUniformLocation(compositeProgram, "depthTex"),0);
        glUniform1i( glGetUniformLocation(compositeProgram, "normalTex"),1);
        glUniform1i( glGetUniformLocation(compositeProgram, "thicknessTex"),2);
        glUniform1i( glGetUniformLocation(compositeProgram, "sceneTex"),3);
        glUniform1f( glGetUniformLocation(compositeProgram, "gamma"),settings->GetSettingAs<float>("thickness_gamma","0.1"));


        //dout<<"Here"<<endl;
        //TODO: should add another shader for performing compositing
        if(material)
        {
            glUniform3fv(glGetUniformLocation(compositeProgram,"material.diffuse"),1,&material->diffuse.x);
            glUniform3fv(glGetUniformLocation(compositeProgram,"material.specular"),1,&material->specular.x);
            glUniform3fv(glGetUniformLocation(compositeProgram,"material.ambient"),1,&material->ambient.x);
            glUniform1fv(glGetUniformLocation(compositeProgram,"material.shininess"),1,&material->shininess);
            glUniform1fv(glGetUniformLocation(compositeProgram,"material.opacity"),1,&material->opacity);
        }
        else
        {
            Material defaultMat;
            defaultMat.ambient=float3(0.1f,0.1f,0.4f);
            defaultMat.diffuse=float3(0.1f,0.1f,0.4f);
            defaultMat.specular=float3(1.0f,1.f,1.0f);
            defaultMat.opacity=0.5;
            defaultMat.shininess=100;
            glUniform3fv(glGetUniformLocation(compositeProgram,"material.diffuse"),1,&defaultMat.diffuse.x);
            glUniform3fv(glGetUniformLocation(compositeProgram,"material.specular"),1,&defaultMat.specular.x);
            glUniform3fv(glGetUniformLocation(compositeProgram,"material.ambient"),1,&defaultMat.ambient.x);
            glUniform1fv(glGetUniformLocation(compositeProgram,"material.shininess"),1,&defaultMat.shininess);
            glUniform1fv(glGetUniformLocation(compositeProgram,"material.opacity"),1,&defaultMat.opacity);
        }
        if(light)
        {
            glUniform3fv(glGetUniformLocation(compositeProgram,"light.diffuse"),1,&light->diffuse.x);
            glUniform3fv(glGetUniformLocation(compositeProgram,"light.specular"),1,&light->specular.x);
            glUniform3fv(glGetUniformLocation(compositeProgram,"light.ambient"),1,&light->ambient.x);
            glUniform3fv(glGetUniformLocation(compositeProgram,"light.position"),1,&light->pos.x);
        }
        else
        {
            Light defaultLight;
            defaultLight.ambient = float3(0.2f,0.2f,0.2f);
            defaultLight.diffuse = float3(1.0f,1.0f,1.0f);
            defaultLight.specular = float3(1.0f,1.0f,1.0f);
            defaultLight.pos = float3(5.0f,10.0f,-5.0f);
            glUniform3fv(glGetUniformLocation(compositeProgram,"light.diffuse"),1,&defaultLight.diffuse.x);
            glUniform3fv(glGetUniformLocation(compositeProgram,"light.specular"),1,&defaultLight.specular.x);
            glUniform3fv(glGetUniformLocation(compositeProgram,"light.ambient"),1,&defaultLight.ambient.x);
            glUniform3fv(glGetUniformLocation(compositeProgram,"light.position"),1,&defaultLight.pos.x);
        }
        RenderUtils::fullscreenQuad();
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D,0);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D,0);
        //glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,framebuffer);


        glDrawBuffer(buffer);
        glActiveTexture(GL_TEXTURE0);
        GLuint copyProgram = m_shaderLibrary->shaders["copyShader"].getProgram();
        glUniform1i( glGetUniformLocation(copyProgram, "colorTex"),0);
        if(settings->GetSettingAs<bool>("render_composite","1"))
        {
            glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs["Color"]);
        }
        else if(settings->GetSettingAs<bool>("render_normal","0"))
        {
            glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs["normalColor"]);
            copyProgram = m_shaderLibrary->shaders["copyInverseShader"].getProgram();
            glUniform1i( glGetUniformLocation(copyProgram, "colorTex"),0);
        }
        else if(settings->GetSettingAs<bool>("render_depth","0"))
        {
            if(currentDepthBuffer=="depth2")
                glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs["depth"]);
            else
                glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs["depth2"]);
            copyProgram = m_shaderLibrary->shaders["copyDepthColorShader"].getProgram();
            glUniform1i( glGetUniformLocation(copyProgram, "scalarTex"),0);
        }
        else if(settings->GetSettingAs<bool>("render_depth_smoothed","0"))
        {
            glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer]);
            copyProgram = m_shaderLibrary->shaders["copyDepthColorShader"].getProgram();
            glUniform1i( glGetUniformLocation(copyProgram, "scalarTex"),0);
        }
        else if(settings->GetSettingAs<bool>("render_thickness","0"))
        {
            glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs["thickness1"]);
            copyProgram = m_shaderLibrary->shaders["copyScalarShader"].getProgram();
            glUniform1i( glGetUniformLocation(copyProgram, "scalarTex"),0);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs["Color"]);
        }
        //need to copy the contents to the back buffer. It's important that we copy the
        //depth as well. Otherwise anything drawn afterwards may incorrecly occlude the fluid.

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer]);

        glUseProgram(copyProgram);

        glUniform1i( glGetUniformLocation(copyProgram, "depthTex"),1);
        RenderUtils::fullscreenQuad();

        glUseProgram(0);
        glBindTexture(GL_TEXTURE_2D,0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,0);

        //if (blending)
        //{
            //glDisable(GL_BLEND);
        //}

        glPopAttrib();
        glPopClientAttrib();
        if (m_writeFramebuffers)
        {
            glFinish();
            writeFramebufferTextures();
            m_writeFramebuffers = false;
        }
    }

    void SSEffect::deleteFramebufferTextures()
    {
        glDeleteTextures(1,&m_glFramebufferTexs["depth"]);
        glDeleteTextures(1,&m_glFramebufferTexs["depth2"]);
        glDeleteTextures(1,&m_glFramebufferTexs["thickness1"]);
        glDeleteTextures(1,&m_glFramebufferTexs["thickness2"]);
#if 0
        glDeleteTextures(1,&m_glFramebufferTexs["depthColor"]);
        glDeleteTextures(1,&m_glFramebufferTexs["depthColorSmooth"]);
#endif
        glDeleteTextures(1,&m_glFramebufferTexs["Color"]);

        glDeleteTextures(1,&m_glFramebufferTexs["normalColor"]);
        glDeleteTextures(1,&m_glFramebufferTexs["composite"]);

    }

    void SSEffect::createFramebufferTextures()
    {
        glPushAttrib(GL_ENABLE_BIT|GL_TEXTURE_BIT);
        glEnable(GL_TEXTURE_2D);
        glGenTextures(1, &m_glFramebufferTexs["depth"]);
        glBindTexture(GL_TEXTURE_2D, m_glFramebufferTexs["depth"]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT32,width,height,0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);
        glGenTextures(1, &m_glFramebufferTexs["depth2"]);
        glBindTexture(GL_TEXTURE_2D, m_glFramebufferTexs["depth2"]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT32,width,height,0,GL_DEPTH_COMPONENT,GL_FLOAT,NULL);
        glGenTextures(1,&m_glFramebufferTexs["thickness1"]);
        glBindTexture(GL_TEXTURE_2D, m_glFramebufferTexs["thickness1"]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
        glGenTextures(1,&m_glFramebufferTexs["thickness2"]);
        glBindTexture(GL_TEXTURE_2D, m_glFramebufferTexs["thickness2"]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);

        glGenTextures(1,&m_glFramebufferTexs["normalColor"]);
        glBindTexture(GL_TEXTURE_2D, m_glFramebufferTexs["normalColor"]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGB32F,width,height,0,GL_RGBA,GL_FLOAT,NULL);
        glGenTextures(1,&m_glFramebufferTexs["composite"]);
        glBindTexture(GL_TEXTURE_2D, m_glFramebufferTexs["composite"]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);

        glGenTextures(1,&m_glFramebufferTexs["Color"]);
        glBindTexture(GL_TEXTURE_2D, m_glFramebufferTexs["Color"]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
#if 0
        glGenTextures(1,&m_glFramebufferTexs["depthColor"]);
        glBindTexture(GL_TEXTURE_2D, m_glFramebufferTexs["depthColor"]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
        glGenTextures(1,&m_glFramebufferTexs["depthColorSmooth"]);
        glBindTexture(GL_TEXTURE_2D, m_glFramebufferTexs["depthColorSmooth"]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
#endif
        glBindTexture(GL_TEXTURE_2D,0);
        glPopAttrib();
    }

    void SSEffect::setWindowDimensions(GLuint width, GLuint height)
    {
        deleteFramebufferTextures();
        ParticleEffect::setWindowDimensions(width,height);
        createFramebufferTextures();
        glEnable(GL_TEXTURE_2D);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,m_fbos[0]);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["thickness1"],0);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["depth"],0);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
        glDisable(GL_TEXTURE_2D);
    }

};
