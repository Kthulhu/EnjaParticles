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
#include <iostream>
#include <vector>
#include <string.h>

#ifdef WIN32
//must include windows.h before gl.h on windows platform
#include <windows.h>
#include <GL/glew.h>
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
#include "../util.h"
#include "Shader.h"
#include "../rtps_common.h"

namespace rtps
{

    /*enum Shaders
    {
        SHADER_DEPTH=0,SHADER_CURVATURE_FLOW,SHADER_FRESNEL
    };*/

    class RTPS_EXPORT ShaderLibrary
    {
    public:
        ShaderLibrary()
        {
        }
        std::map<std::string,Shader> shaders;
        void initializeShaders(std::string shaderSrc)
        {
            std::string vert,frag,geom,tessCont,tessEval;
			vert = "";
			frag = "";
            geom = "";
            tessCont="";
            tessEval="";
            readFile(shaderSrc+"/sphere.vert",vert);
            readFile(shaderSrc+"/sphere.frag",frag);
            addShader("sphereShader",vert,frag,geom,tessCont,tessEval);
            vert = "";
            frag = "";
            readFile(shaderSrc+"/passthrough.vert",vert);
            readFile(shaderSrc+"/passthrough.frag",frag);
            addShader("passThrough",vert,frag,geom,tessCont,tessEval);
            //frag="";
            //readFile(shaderSrc+"/sphere_light.glsl",frag);
            //addShader("sphereLightShader",vert,frag,geom,tessCont,tessEval);
            //vert = frag = "";
            //readFile(shaderSrc+"/depth.vert",vert);
            //readFile(shaderSrc+"/depth_frag.glsl",frag);
            //addShader("depthShader",vert,frag,geom,tessCont,tessEval);
            vert = frag = "";
            readFile(shaderSrc+"/post_process.vert",vert);
            readFile(shaderSrc+"/gaussian_blur_x.frag",frag);
            addShader("gaussBlurXShader",vert,frag,geom,tessCont,tessEval);
            frag = "";
            readFile(shaderSrc+"/gaussian_blur_y.frag",frag);
            addShader("gaussBlurYShader",vert,frag,geom,tessCont,tessEval);
            frag = "";
            readFile(shaderSrc+"/bilateral_blur.frag",frag);
            addShader("bilateralGaussianBlurShader",vert,frag,geom,tessCont,tessEval);
            //frag = "";
            //readFile(shaderSrc+"/composite_screen_space.frag",frag);
            //addShader("compositeScreenSpace",vert,frag,geom,tessCont,tessEval);
            frag = "";
            readFile(shaderSrc+"/light_post_process.frag",frag);
            addShader("lightPostProcess",vert,frag,geom,tessCont,tessEval);
            vert = frag = "";
            readFile(shaderSrc+"/normal.vert",vert);
            readFile(shaderSrc+"/normal.frag",frag);
            addShader("depth2NormalShader",vert,frag,geom,tessCont,tessEval);
            vert = frag = "";
            readFile(shaderSrc+"/copy.vert",vert);
            readFile(shaderSrc+"/copy.frag",frag);
            addShader("copyShader",vert,frag,geom,tessCont,tessEval);
            vert = frag = "";
            readFile(shaderSrc+"/draw_vector.vert",vert);
            readFile(shaderSrc+"/draw_vector.frag",frag);
            readFile(shaderSrc+"/draw_vector.geom",geom);
            shaders["vectorShader"].attachGeometryParam(GL_GEOMETRY_INPUT_TYPE_EXT,GL_POINTS);
            shaders["vectorShader"].attachGeometryParam(GL_GEOMETRY_OUTPUT_TYPE_EXT,GL_LINE_STRIP);
            addShader("vectorShader",vert,frag,geom,tessCont,tessEval);
            vert = frag = geom="";
            readFile(shaderSrc+"/render_instanced.vert",vert);
            readFile(shaderSrc+"/render_instanced.frag",frag);
            addShader("renderInstancedShader",vert,frag,geom,tessCont,tessEval);
            vert = frag = geom="";
            readFile(shaderSrc+"/render_lit.vert",vert);
            readFile(shaderSrc+"/render_lit.frag",frag);
            addShader("renderLitShader",vert,frag,geom,tessCont,tessEval);
            vert = frag = geom="";
            readFile(shaderSrc+"/render_water_IBL.vert",vert);
            readFile(shaderSrc+"/render_water_IBL.frag",frag);
            addShader("renderFluidShader",vert,frag,geom,tessCont,tessEval);
            //readFile(shaderSrc+"/renderInstanced_geom.glsl",geom);
        }

        void addShader(const std::string& name, const std::string& vert,
                        const std::string& frag, const std::string& geom = "",
                        const std::string& tessCont = "", const std::string& tessEval = "")
        {
            shaders[name].setShader(GL_VERTEX_SHADER,vert);
            shaders[name].setShader(GL_FRAGMENT_SHADER,frag);
			#ifdef GL_GEOMETERY_SHADER
            shaders[name].setShader(GL_GEOMETRY_SHADER,geom);
			#endif
#ifdef GL_TESS_CONTROL_SHADER
            shaders[name].setShader(GL_TESS_CONTROL_SHADER,tessCont);
            shaders[name].setShader(GL_TESS_EVALUATION_SHADER,tessEval);
#endif
            //dout<<"Program = "<<shaders[name].compileProgram()<<std::endl;
        }
    };
}

#endif
