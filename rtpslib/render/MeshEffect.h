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


#ifndef MESHEFFECT_H
#define MESHEFFECT_H
#include <map>
#include <vector>
#include <string.h>

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


#include <assimp/assimp.h>

#include "ParticleEffect.h"
#include "../structs.h"
#include "../timer_eb.h"
#include "ShaderLibrary.h"

#include "../rtps_common.h"

namespace rtps
{

    struct RTPS_EXPORT Mesh
    {
        float16 modelMat;
        GLuint vbo;
        GLuint colbo;
        GLuint normalbo;
        GLuint ibo;
        GLuint texCoordsbo;
        GLuint tex;
        bool hasNormals;
        bool hasTexture;
        unsigned int vboSize;
        unsigned int iboSize;
        Material material;
        GLenum drawMode;
	Mesh()
	{
		vbo=0;colbo=0;normalbo=0;ibo=0;
		texCoordsbo=0;tex=0;hasNormals=false;
		hasTexture=false; vboSize=0;
        iboSize=0;drawMode=GL_TRIANGLES;
	}
    };
    class MeshEffect : public ParticleEffect
    {
    public:
        MeshEffect(ShaderLibrary* lib, GLuint width = 600, GLuint height = 800,GLfloat pointRadius = 0.5f,bool blending = false);
        ~MeshEffect();
        void renderFluid(Mesh* mesh, GLuint cubeMap, GLuint sceneTex, Light* light);
        virtual void renderInstanced(Mesh* mesh, GLuint pos, GLuint quat,unsigned int number,Light* light);
        virtual void render(Mesh* mesh, Light* light);
    };
};

#endif
