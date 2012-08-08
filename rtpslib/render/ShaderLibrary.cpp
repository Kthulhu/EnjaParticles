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


#ifdef WIN32
//must include windows.h before gl.h on windows platform
#include <windows.h>
#endif

#include <GL/glew.h>
#if defined __APPLE__ || defined(MACOSX)
//OpenGL stuff
    #include <OpenGL/glu.h>
    #include <OpenGL/gl.h>
#else
//OpenGL stuff
    #include <GL/glu.h>
    #include <GL/gl.h>
#endif

#include "ShaderLibrary.h"

namespace rtps
{

        void ShaderLibrary::initializeShaders(std::string shaderSrc)
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
            frag = "";
            readFile(shaderSrc+"/sphere_thickness.frag",frag);
            addShader("sphereThicknessShader",vert,frag,geom,tessCont,tessEval);
            vert = "";
            frag = "";
            readFile(shaderSrc+"/passthrough.vert",vert);
            readFile(shaderSrc+"/passthrough.frag",frag);
            addShader("passThrough",vert,frag,geom,tessCont,tessEval);
            vert = "";
            frag = "";
            readFile(shaderSrc+"/skybox.vert",vert);
            readFile(shaderSrc+"/skybox.frag",frag);
            addShader("skybox",vert,frag,geom,tessCont,tessEval);
            //frag="";
            //readFile(shaderSrc+"/sphere_light.glsl",frag);
            //addShader("sphereLightShader",vert,frag,geom,tessCont,tessEval);
            //vert = frag = "";
            //readFile(shaderSrc+"/depth.vert",vert);
            //readFile(shaderSrc+"/depth_frag.glsl",frag);
            //addShader("depthShader",vert,frag,geom,tessCont,tessEval);
            vert = frag = "";
            readFile(shaderSrc+"/post_process.vert",vert);
            readFile(shaderSrc+"/gaussian_blur.frag",frag);
            addShader("gaussianBlurShader",vert,frag,geom,tessCont,tessEval);
            frag = "";
            readFile(shaderSrc+"/gaussian_blur_x.frag",frag);
            addShader("gaussianBlurXShader",vert,frag,geom,tessCont,tessEval);
            frag = "";
            readFile(shaderSrc+"/gaussian_blur_y.frag",frag);
            addShader("gaussianBlurYShader",vert,frag,geom,tessCont,tessEval);
            frag = "";
            readFile(shaderSrc+"/bilateral_blur.frag",frag);
            addShader("bilateralGaussianBlurShader",vert,frag,geom,tessCont,tessEval);
            frag = "";
            readFile(shaderSrc+"/curvature_flow.frag",frag);
            addShader("curvatureFlowShader",vert,frag,geom,tessCont,tessEval);
            //frag = "";
            //readFile(shaderSrc+"/composite_screen_space.frag",frag);
            //addShader("compositeScreenSpace",vert,frag,geom,tessCont,tessEval);
            //frag = "";
            //readFile(shaderSrc+"/light_post_process.frag",frag);
            //addShader("lightPostProcess",vert,frag,geom,tessCont,tessEval);
            frag = "";
            readFile(shaderSrc+"/normal.frag",frag);
            addShader("depth2NormalShader",vert,frag,geom,tessCont,tessEval);
            frag = "";
            readFile(shaderSrc+"/composite_fluid.frag",frag);
            addShader("compositeFluidShader",vert,frag,geom,tessCont,tessEval);
            frag = "";
            readFile(shaderSrc+"/copy.frag",frag);
            addShader("copyShader",vert,frag,geom,tessCont,tessEval);
            frag = "";
            readFile(shaderSrc+"/copy_scalar.frag",frag);
            addShader("copyScalarShader",vert,frag,geom,tessCont,tessEval);
            frag = "";
            readFile(shaderSrc+"/copy_inverse.frag",frag);
            addShader("copyInverseShader",vert,frag,geom,tessCont,tessEval);
            frag = "";
            readFile(shaderSrc+"/copy_depth_color.frag",frag);
            addShader("copyDepthColorShader",vert,frag,geom,tessCont,tessEval);
            frag = "";
            readFile(shaderSrc+"/fixed_width_gaussian.frag",frag);
            addShader("fixedWidthGaussianShader",vert,frag,geom,tessCont,tessEval);
            vert = frag ="";
            readFile(shaderSrc+"/draw_streamline.vert",vert);
            readFile(shaderSrc+"/draw_streamline.frag",frag);
            addShader("streamlineShader",vert,frag,geom,tessCont,tessEval);
            vert = frag = geom="";
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

        void ShaderLibrary::addShader(const std::string& name, const std::string& vert,
                        const std::string& frag, const std::string& geom,
                        const std::string& tessCont, const std::string& tessEval )
        {
            shaders[name].setShader(GL_VERTEX_SHADER,vert);
            shaders[name].setShader(GL_FRAGMENT_SHADER,frag);
            //dout<<"about to say geometry shader is defined"<<std::endl;
            #ifdef GL_GEOMETRY_SHADER
            //dout<<"Geometry shader is defined"<<std::endl;
            shaders[name].setShader(GL_GEOMETRY_SHADER,geom);
			#endif
#ifdef GL_TESS_CONTROL_SHADER
            shaders[name].setShader(GL_TESS_CONTROL_SHADER,tessCont);
            shaders[name].setShader(GL_TESS_EVALUATION_SHADER,tessEval);
#endif
	    GLuint program = shaders[name].compileProgram();
            dout<<"Name = "<<name<<" Program = "<<program<<std::endl;
        }
}

