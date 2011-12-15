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


#ifndef RTPS_SHADER_LIBRARY_H_INCLUDED
#define RTPS_SHADER_LIBRARY_H_INCLUDED

#include <map>
#include <vector>
#include <string.h>

struct RenderSettings
{
    std::string shaderPath;
    GLuint windowHeight,windowWidth;
    float particleRadius;
    float nearDepth;
    float farDepth;
    bool blending;
};
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

    /*enum Shaders
    {
        SHADER_DEPTH=0,SHADER_CURVATURE_FLOW,SHADER_FRESNEL
    };*/

    struct ShaderLibrary 
    {
        static std::map<std::string,Shader> shaders;    
        static void initializeShaders(std::string shaderSrc)
        {
            std::string vert,frag,geom,tessCont,tessEval;
            geom = "";
            tessCont="";
            tessEval="";
            readFile(shaderSrc+"/sphere_vert.glsl",vert);
            readFile(shaderSrc+"/sphere_frag.glsl",frag);
            addShader("sphereShader",vert,frag,geom,tessCont,tessEval);
            readFile(shaderSrc+"/depth_vert.glsl",vert);
            readFile(shaderSrc+"/depth_frag.glsl",frag);
            addShader("depthShader",vert,frag,geom,tessCont,tessEval);
            readFile(shaderSrc+"/gaussian_blur_vert.glsl",vert);
            readFile(shaderSrc+"/gaussian_blur_x_frag.glsl",frag);
            addShader("gaussianBlurXShader",vert,frag,geom,tessCont,tessEval);
            readFile(shaderSrc+"/gaussian_blur_y_frag.glsl",frag);
            addShader("gaussianBlurYShader",vert,frag,geom,tessCont,tessEval);
            readFile(shaderSrc+"/gaussian_bilateral_frag.glsl",frag);
            addShader("bilateralGaussianBlurShader",vert,frag,geom,tessCont,tessEval);
            readFile(shaderSrc+"/normal_vert.glsl",vert);
            readFile(shaderSrc+"/normal_frag.glsl",frag);
            addShader("depth2NormalShader",vert,frag,geom,tessCont,tessEval);
            readFile(shaderSrc+"/copy_vert.glsl",vert);
            readFile(shaderSrc+"/copy_frag.glsl",frag);
            addShader("copyWDepthTestShader",vert,frag,geom,tessCont,tessEval);
            readFile(shaderSrc+"/vector_vert.glsl",vert);
            readFile(shaderSrc+"/vector_frag.glsl",frag);
            readFile(shaderSrc+"/vector_geom.glsl",geom);
            shaders["vectorShader"].attachGeometryParam(GL_GEOMETRY_INPUT_TYPE_EXT,GL_POINTS);
            shaders["vectorShader"].attachGeometryParam(GL_GEOMETRY_OUTPUT_TYPE_EXT,GL_LINE_STRIP);
            addShader("vectorShader",vert,frag,geom,tessCont,tessEval);
        }

        void addShader(const std::string& name, const std::string& vert,
                        const std::string& frag, const std::string& geom = "",
                        const std::string& tessCont = "", const std::string& tessEval = "")
        {
            shaders[name].setShader(GL_VERTEX_SHADER,vert);
            shaders[name].setShader(GL_FRAGMENT_SHADER,frag);
            shaders[name].setShader(GL_GEOMETRY_SHADER,geom);
            shaders[name].setShader(GL_TESS_CONTROL_SHADER,tessCont);
            shaders[name].setShader(GL_TESS_EVALUATION_SHADER,tessEval);
            shaders[name].compileProgram();
        }
    };  
}

#endif
