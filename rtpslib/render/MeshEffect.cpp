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



#include <GL/glew.h>
#include "MeshEffect.h"

using namespace std;
namespace rtps
{
    MeshEffect::MeshEffect(RenderSettings set, ShaderLibrary& lib):ParticleEffect(set,lib)
    {}
    MeshEffect::~MeshEffect(){}
    void MeshEffect::render(Mesh* mesh)
    {
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glMultMatrixf((float*)&mesh->modelMat);
        glEnableVertexAttribArray(0);
        //glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,0);
        //glBindBuffer(GL_ARRAY_BUFFER, mesh->colbo);
        //glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,0,0);
        if(mesh->hasNormals)
        {
            glEnableVertexAttribArray(2);
            glBindBuffer(GL_ARRAY_BUFFER, mesh->normalbo);
            glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,0,0);
            glEnable(GL_LIGHTING);
        }
        else
        {
            glDisable(GL_LIGHTING);
        }
        if(mesh->hasTexture)
        {
            dout<<"Here"<<endl;
            glEnableVertexAttribArray(3);
            glBindBuffer(GL_ARRAY_BUFFER, mesh->texCoordsbo);
            glVertexAttribPointer(3,2,GL_FLOAT,GL_FALSE,0,0);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,mesh->tex);
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
        //TODO: Create My own matrix class to handle this. Or use boost. That way
        //I can be opengl 3+ compliant.
        float16 modelview;
        glGetFloatv(GL_MODELVIEW_MATRIX,modelview.m);
        float16 project;
        glGetFloatv(GL_PROJECTION_MATRIX,project.m);
        glUniformMatrix4fv(glGetUniformLocation(m_shaderLibrary.shaders["renderInstancedShader"].getProgram(),"modelview"),1,true,modelview.m);
        glUniformMatrix4fv(glGetUniformLocation(m_shaderLibrary.shaders["renderInstancedShader"].getProgram(),"project"),1,true,project.m);
        glUseProgram(m_shaderLibrary.shaders["renderLitShader"].getProgram());

        modelview.print("modelview");
        project.print("projection");
        dout<<"ibo "<<mesh->ibo<<endl;
        dout<<"ibosize "<<mesh->iboSize<<endl;
        dout<<"vbo "<<mesh->vbo<<endl;
        dout<<"vbosize "<<mesh->vboSize<<endl;
        dout<<"normals "<<mesh->normalbo<<endl;
        dout<<"texcoords "<<mesh->texCoordsbo<<endl;
        glDrawElements(GL_TRIANGLES,mesh->iboSize,GL_UNSIGNED_INT,0);
        glUseProgram(0);
        glDisableVertexAttribArray(0);
        //glDisableVertexAttribArray(1);
        if(mesh->hasNormals)
        {
            glDisableVertexAttribArray(2);
            glDisable(GL_LIGHTING);
        }
        if(mesh->hasTexture)
        {
            glDisableVertexAttribArray(3);
            glDisable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,0);
        }
        glPopMatrix();
    }
    void MeshEffect::renderInstanced(Mesh* mesh, GLuint pos, GLuint quat, unsigned int size)
    {
        glEnableVertexAttribArray(0);
        glVertexAttribDivisorARB(0,0);
        glEnableVertexAttribArray(1);
        glVertexAttribDivisorARB(1,0);
        glEnableVertexAttribArray(2);
        glVertexAttribDivisorARB(2,1);
        glEnableVertexAttribArray(3);
        glVertexAttribDivisorARB(3,1);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,0);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->colbo);
        glVertexAttribPointer(1,4,GL_FLOAT,GL_FALSE,0,0);
        glBindBuffer(GL_ARRAY_BUFFER, pos);
        glVertexAttribPointer(2,4,GL_FLOAT,GL_FALSE,0,0);
        glBindBuffer(GL_ARRAY_BUFFER, quat);
        glVertexAttribPointer(3,4,GL_FLOAT,GL_FALSE,0,0);
        if(mesh->hasNormals)
        {
            glEnableVertexAttribArray(4);
            glVertexAttribDivisorARB(4,0);
            glBindBuffer(GL_ARRAY_BUFFER, mesh->normalbo);
            glVertexAttribPointer(4,3,GL_FLOAT,GL_FALSE,0,0);
            glEnable(GL_LIGHTING);
        }
        else
        {
            glDisable(GL_LIGHTING);
        }
        if(mesh->hasTexture)
        {
            glEnableVertexAttribArray(5);
            glVertexAttribDivisorARB(5,0);
            glBindBuffer(GL_ARRAY_BUFFER, mesh->texCoordsbo);
            glVertexAttribPointer(5,2,GL_FLOAT,GL_FALSE,0,0);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,mesh->tex);
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ibo);
        glUseProgram(m_shaderLibrary.shaders["renderInstancedShader"].getProgram());
        //TODO: Create My own matrix class to handle this. Or use boost. That way
        //I can be opengl 3+ compliant.
        float16 modelview;
        glGetFloatv(GL_MODELVIEW_MATRIX,modelview.m);
        float16 project;
        glGetFloatv(GL_PROJECTION_MATRIX,project.m);
        glUniformMatrix4fv(glGetUniformLocation(m_shaderLibrary.shaders["renderInstancedShader"].getProgram(),"modelview"),1,true,modelview.m);
        glUniformMatrix4fv(glGetUniformLocation(m_shaderLibrary.shaders["renderInstancedShader"].getProgram(),"project"),1,true,project.m);
        glDrawElementsInstancedEXT(GL_TRIANGLES,mesh->iboSize,GL_UNSIGNED_INT,0,size);
        glUseProgram(0);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
        glDisableVertexAttribArray(3);
        if(mesh->hasNormals)
        {
            glDisableVertexAttribArray(4);
            glDisable(GL_LIGHTING);
        }
        if(mesh->hasTexture)
        {
            glDisableVertexAttribArray(5);
            glDisable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,0);
        }
    }
}
