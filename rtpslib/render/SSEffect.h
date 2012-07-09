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


#ifndef SS_EFFECT_H
#define SS_EFFECT_H

#include "ParticleEffect.h"
#include "../rtps_common.h"

namespace rtps
{
    enum SmoothingFilter
    {
        NO_SMOOTHING = 0,
        GUASSIAN_BLUR,
        SEPERABLE_GAUSS_BLUR,
        BILATERAL_GAUSSIAN_BLUR,
        CURVATURE_FLOW
    };
    class RTPS_EXPORT SSEffect : public ParticleEffect
    {
    public:
        SSEffect(ShaderLibrary* lib, SmoothingFilter filter = SmoothingFilter::BILATERAL_GAUSSIAN_BLUR, GLuint width = 600, GLuint height = 800, GLfloat near=0.0f, GLfloat far =1.0f,GLfloat pointRadius = 0.5f,bool blending = false);
        ~SSEffect();
        void smoothDepth();
        virtual void render(GLuint posVBO, GLuint colVBO, unsigned int num, const Light* light = NULL,const Material* material = NULL, float scale =1.0f);
        virtual void setWindowDimensions(GLuint width, GLuint height);
        virtual void setSmoothingFilter(SmoothingFilter filter){this->smoothing=filter;}
        virtual SmoothingFilter getSmoothingFilter(){return smoothing;}
        virtual void setRenderThickness(bool thickness){this->thickness = thickness;}
        virtual bool getRenderThickness(){return thickness;}
        virtual void setNumberOfCurvatureIterations(unsigned int num){numberOfCurvatureIterations=num;}
        virtual unsigned int getNumberOfCurvatureIterations(){return numberOfCurvatureIterations;}
        virtual void setFilterRadius(unsigned int filterRadius){this->filterRadius=filterRadius;}
        virtual unsigned int getFilterRadius(){return this->filterRadius;}
    protected:
        virtual void deleteFramebufferTextures();
        virtual void createFramebufferTextures();
        bool thickness;
        unsigned int filterRadius;
        unsigned int numberOfCurvatureIterations;
        std::string currentDepthBuffer;
        SmoothingFilter smoothing;
    };
};

#endif
