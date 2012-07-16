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
    SSEffect::SSEffect(ShaderLibrary* lib, SmoothingFilter filter, GLuint width, GLuint height, GLfloat pointRadius,bool blending):
        ParticleEffect(lib,width,height,pointRadius,blending)
    {
        //cout<<"Shaderlib size = "<<m_shaderLibrary.shaders.size()<<endl;
        m_fbos.resize(1);
        glGenFramebuffersEXT(1,&m_fbos[0]);
        createFramebufferTextures();
        smoothing=filter;
        currentDepthBuffer="depth";
        bilateralRange=0.01f;
        filterRadius=8.0f;
        falloff=0.001f;
        numberOfCurvatureIterations=50;
        thickness=false;
        //dout<<"width = "<<width<<"height = "<<height<<endl;
        glEnable(GL_TEXTURE_2D);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,m_fbos[0]);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["thickness"],0);

        //glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT,m_fbos[0]);
        //glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["thickness"],0);
        //glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT1_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["depthColor"],0);
        //glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT2_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["normalColor"],0);
        //glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT3_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["lightColor"],0);
        //glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT4_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["Color"],0);
        //glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT5_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["depthColorSmooth"],0);
        //glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["depth"],0);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["depth"],0);

        dout<<"fbo[0] = "<<m_fbos[0]<<" status complete? "<<((glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE)?"yes":"no")<<" "<<glCheckFramebufferStatus(GL_FRAMEBUFFER)<<endl;
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
        //glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT,0);

        glDisable(GL_TEXTURE_2D);
    }
    void SSEffect::smoothDepth()
    {
        GLuint smoothingProgram;
        switch(smoothing)
        {
            case NO_SMOOTHING:
                currentDepthBuffer="depth";
                return;
            case SEPERABLE_GAUSSIAN_BLUR:
                smoothingProgram= m_shaderLibrary->shaders["gaussianBlurXShader"].getProgram();
                glUseProgram(smoothingProgram);
                glUniform1i( glGetUniformLocation(smoothingProgram, "depthTex"),0);
                glUniform1f( glGetUniformLocation(smoothingProgram, "del_x"),1.0f/(float)width);
                glUniform1f( glGetUniformLocation(smoothingProgram, "falloff"),falloff);
                glUniform1f( glGetUniformLocation(smoothingProgram, "sig"),filterRadius);
                RenderUtils::fullscreenQuad();

                glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer]);
                currentDepthBuffer="depth";
                glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer],0);

                //glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer],0);
                glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
                smoothingProgram= m_shaderLibrary->shaders["gaussianBlurYShader"].getProgram();
                glUseProgram(smoothingProgram);
                glUniform1i( glGetUniformLocation(smoothingProgram, "depthTex"),0);
                glUniform1f( glGetUniformLocation(smoothingProgram, "del_y"),1.0f/(float)height);
                glUniform1f( glGetUniformLocation(smoothingProgram, "falloff"),falloff);
                glUniform1f( glGetUniformLocation(smoothingProgram, "sig"),filterRadius);
                currentDepthBuffer="depth2";
                break;
            case GAUSSIAN_BLUR:
                smoothingProgram= m_shaderLibrary->shaders["gaussianBlurShader"].getProgram();
                glUseProgram(smoothingProgram);
                glUniform1i(glGetUniformLocation(smoothingProgram,"depthTex"),0);
                glUniform1f( glGetUniformLocation(smoothingProgram, "del_x"),1.0/((float)width));
                glUniform1f( glGetUniformLocation(smoothingProgram, "del_y"),1.0/((float)height));
                glUniform1f( glGetUniformLocation(smoothingProgram, "falloff"),falloff);
                glUniform1f( glGetUniformLocation(smoothingProgram, "sig"),filterRadius);
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
                glUniform1f( glGetUniformLocation(smoothingProgram, "sig_range"),bilateralRange);
                glUniform1f( glGetUniformLocation(smoothingProgram, "sig"),filterRadius);
                RenderUtils::fullscreenQuad();
                glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer],0);

                //glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer],0);
                glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer]);
                currentDepthBuffer="depth";
                glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

                glUniform2fv( glGetUniformLocation(smoothingProgram, "blurDir"),1,ydir);
                glUniform1f( glGetUniformLocation(smoothingProgram, "sig_range"),bilateralRange);
                //glUniform1f( glGetUniformLocation(glsl_program[BILATERAL_GAUSSIAN_SHADER], "sig"),filterRadius);
                currentDepthBuffer="depth2";
                break;
            }
            case CURVATURE_FLOW:
            {
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
                glUniform1f( glGetUniformLocation(smoothingProgram, "dt"),1.0f/numberOfCurvatureIterations);
                glUniform1f( glGetUniformLocation(smoothingProgram, "distance_threshold"), falloff);

                for(unsigned int i = 0; i<numberOfCurvatureIterations; i++)
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
                return;
            }
            default:
                break;
        }
        RenderUtils::fullscreenQuad();
    }

    void SSEffect::render(GLuint posVBO, GLuint colVBO, unsigned int num, const Light* light,const Material* material, float scale)
    {
        if(num==0)
            return;
        //dout<<"Here"<<endl;
        //perserve original buffer
        GLint buffer;
        glGetIntegerv(GL_DRAW_BUFFER,&buffer);

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,m_fbos[0]);
        //glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT,m_fbos[0]);

        glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D,0);
        //Should probably conditionally create the thickness buffer as well.
        //Render Thickness buffer.
        if(thickness)
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE);
            //glDrawBuffers(2,buffers);
            glDisable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["thickness"],0);
            //glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
            glClearColor(0.0f,0.0f,0.0f,0.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            renderPointsAsSpheres(posVBO, colVBO, num, light,material,scale);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND);
        }

        glEnable(GL_DEPTH_TEST);
        //dout<<"Here"<<endl;
        //Render Color and depth buffer of spheres.
        //glDrawBuffer(GL_COLOR_ATTACHMENT4_EXT);
        //glClearColor(0.0f,0.0f,0.0f,1.0f);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["Color"],0);
        //glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["Color"],0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderPointsAsSpheres(posVBO, colVBO, num, light,material,scale);
        //smoothDepth();
        //dout<<"Here"<<endl;


        //Smooth the depth texture to emulate a surface.
        //glDrawBuffer(GL_COLOR_ATTACHMENT1);
        //glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
        //glDrawBuffer(GL_COLOR_ATTACHMENT5_EXT);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["depthColorSmooth"],0);

        //glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["depthColorSmooth"],0);
        glBindTexture(GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer]);
        currentDepthBuffer="depth2";
        //glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer],0);

        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer],0);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        //dout<<"Here"<<endl;

        smoothDepth();
        //dout<<"Here"<<endl;
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer],0);

        //glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_glFramebufferTexs[currentDepthBuffer],0);
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
        //glDrawBuffer(GL_COLOR_ATTACHMENT2_EXT);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["normalColor"],0);

        //glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["normalColor"],0);
        //glClearColor(0.0f,0.0f,0.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        GLuint normalProgram = m_shaderLibrary->shaders["depth2NormalShader"].getProgram();
        glUseProgram(normalProgram);
        glUniform1i( glGetUniformLocation(normalProgram, "depthTex"),0);
        //glUniform1i( glGetUniformLocation(normalProgram, "colorTex"),1);
        glUniform1f( glGetUniformLocation(normalProgram, "del_x"),1.0/((float)width));
        glUniform1f( glGetUniformLocation(normalProgram, "del_y"),1.0/((float)height));
        if(material)
        {
            glUniform3fv(glGetUniformLocation(normalProgram,"material.diffuse"),1,&material->diffuse.x);
            glUniform3fv(glGetUniformLocation(normalProgram,"material.specular"),1,&material->specular.x);
            glUniform3fv(glGetUniformLocation(normalProgram,"material.ambient"),1,&material->ambient.x);
            glUniform1fv(glGetUniformLocation(normalProgram,"material.shininess"),1,&material->shininess);
            glUniform1fv(glGetUniformLocation(normalProgram,"material.opacity"),1,&material->opacity);
        }
        else
        {
            Material defaultMat;
            defaultMat.ambient=float3(0.0f,0.2f,0.6f);
            defaultMat.diffuse=float3(0.0f,0.2f,0.6f);
            defaultMat.specular=float3(1.0f,1.f,1.0f);
            defaultMat.opacity=0.6;
            defaultMat.shininess=100;
            glUniform3fv(glGetUniformLocation(normalProgram,"material.diffuse"),1,&defaultMat.diffuse.x);
            glUniform3fv(glGetUniformLocation(normalProgram,"material.specular"),1,&defaultMat.specular.x);
            glUniform3fv(glGetUniformLocation(normalProgram,"material.ambient"),1,&defaultMat.ambient.x);
            glUniform1fv(glGetUniformLocation(normalProgram,"material.shininess"),1,&defaultMat.shininess);
            glUniform1fv(glGetUniformLocation(normalProgram,"material.opacity"),1,&defaultMat.opacity);
        }
        if(light)
        {
            glUniform3fv(glGetUniformLocation(normalProgram,"light.diffuse"),1,&light->diffuse.x);
            glUniform3fv(glGetUniformLocation(normalProgram,"light.specular"),1,&light->specular.x);
            glUniform3fv(glGetUniformLocation(normalProgram,"light.ambient"),1,&light->ambient.x);
            glUniform3fv(glGetUniformLocation(normalProgram,"light.position"),1,&light->pos.x);
        }
        else
        {
            Light defaultLight;
            defaultLight.ambient = float3(1.0f,1.0f,1.0f);
            defaultLight.diffuse = float3(1.0f,1.0f,1.0f);
            defaultLight.specular = float3(1.0f,1.0f,1.0f);
            defaultLight.pos = float3(5.0f,5.0f,-5.0f);
            glUniform3fv(glGetUniformLocation(normalProgram,"light.diffuse"),1,&defaultLight.diffuse.x);
            glUniform3fv(glGetUniformLocation(normalProgram,"light.specular"),1,&defaultLight.specular.x);
            glUniform3fv(glGetUniformLocation(normalProgram,"light.ambient"),1,&defaultLight.ambient.x);
            glUniform3fv(glGetUniformLocation(normalProgram,"light.position"),1,&defaultLight.pos.x);
        }
        RenderUtils::fullscreenQuad();
        //dout<<"Here"<<endl;
        //TODO: should add another shader for performing compositing

        /*
        glUseProgram(0);
        glBindTexture(GL_TEXTURE_2D,0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,0);
        */

        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);

        //glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT,0);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);

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

        if (blending)
        {
            glDisable(GL_BLEND);
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDisable(GL_TEXTURE_2D);
        //printf("done rendering\n");
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
        glDeleteTextures(1,&m_glFramebufferTexs["thickness"]);
        glDeleteTextures(1,&m_glFramebufferTexs["depthColor"]);
        glDeleteTextures(1,&m_glFramebufferTexs["depthColorSmooth"]);
        glDeleteTextures(1,&m_glFramebufferTexs["normalColor"]);
        glDeleteTextures(1,&m_glFramebufferTexs["lightColor"]);
        glDeleteTextures(1,&m_glFramebufferTexs["Color"]);
    }

    void SSEffect::createFramebufferTextures()
    {
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
        glGenTextures(1,&m_glFramebufferTexs["thickness"]);
        glBindTexture(GL_TEXTURE_2D, m_glFramebufferTexs["thickness"]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
        //glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA32F,width,height,0,GL_RGBA,GL_FLOAT,NULL);
        glGenTextures(1,&m_glFramebufferTexs["depthColor"]);
        glBindTexture(GL_TEXTURE_2D, m_glFramebufferTexs["depthColor"]);
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
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
        //glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA32F,width,height,0,GL_RGBA,GL_FLOAT,NULL);
        glGenTextures(1,&m_glFramebufferTexs["lightColor"]);
        glBindTexture(GL_TEXTURE_2D, m_glFramebufferTexs["lightColor"]);
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
        glGenTextures(1,&m_glFramebufferTexs["depthColorSmooth"]);
        glBindTexture(GL_TEXTURE_2D, m_glFramebufferTexs["depthColorSmooth"]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,width,height,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);
        //glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA32F,width,height,0,GL_RGBA,GL_FLOAT,NULL);
        glBindTexture(GL_TEXTURE_2D,0);
        glDisable(GL_TEXTURE_2D);

    }

    void SSEffect::setWindowDimensions(GLuint width, GLuint height)
    {
        deleteFramebufferTextures();
        ParticleEffect::setWindowDimensions(width,height);
        createFramebufferTextures();
        glEnable(GL_TEXTURE_2D);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,m_fbos[0]);

        //glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT,m_fbos[0]);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["thickness"],0);

        //glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["thickness"],0);
        //glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT1_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["depthColor"],0);
        //glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT2_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["normalColor"],0);
        //glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT3_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["lightColor"],0);
        //glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT4_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["Color"],0);
        //glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT5_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["depthColorSmooth"],0);
        //glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["depth"],0);
        //glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT,0);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,m_glFramebufferTexs["depth"],0);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
        glDisable(GL_TEXTURE_2D);
    }

};
