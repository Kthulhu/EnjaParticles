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


#include <iostream>
#include <string.h>

#include <GL/glew.h>

#include "RenderUtils.h"
#include "util.h"
#include "stb_image.h" 
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h" 

using namespace std;

namespace rtps
{
    
    void RenderUtils::orthoProjection()
    {
        glMatrixMode(GL_PROJECTION);                    // Select Projection
        glPushMatrix();                         // Push The Matrix
        glLoadIdentity();                       // Reset The Matrix
        gluOrtho2D( 0,1,0,1);
        glMatrixMode(GL_MODELVIEW);                 // Select Modelview Matrix
        glPushMatrix();                         // Push The Matrix
        glLoadIdentity();                       // Reset The Matrix
    }

    void RenderUtils::perspectiveProjection()
    {
        glMatrixMode( GL_PROJECTION );                  // Select Projection
        glPopMatrix();                          // Pop The Matrix
        glMatrixMode( GL_MODELVIEW );                   // Select Modelview
        glPopMatrix();                          // Pop The Matrix
    }

    void RenderUtils::fullscreenQuad()
    {
        orthoProjection();
        glBegin(GL_QUADS);
        glTexCoord2f(0.f,0.f);
        glVertex2f(0.f,0.f);

        glTexCoord2f(1.f,0.f);
        glVertex2f(1.f,0.f);

        glTexCoord2f(1.f,1.f);
        glVertex2f(1.f,1.f);

        glTexCoord2f(0.f,1.f);
        glVertex2f(0.f,1.f);
        glEnd();
        perspectiveProjection();
    }
    void RenderUtils::writeTextures(const map<string,GLuint>& texs) 
    {
        for (map<string,GLuint>::const_iterator i = texs.begin();i!=texs.end();i++)
        {
            string s(i->first);
            s+=".png";
            //Fixme: This is a hacky way to find out if the texture is Depth.
            //I should probably query GL for info about the texture.
            writeTexture(i->second, s, i->first.find("depth"));
        }
    }
    int RenderUtils::writeTexture( GLuint tex, const string& filename, bool depth)
    {
        glBindTexture(GL_TEXTURE_2D,tex);
        GLint width,height;
        glGetTexLevelParameteriv(tex , 0 , GL_TEXTURE_WIDTH , &width);
        glGetTexLevelParameteriv(tex , 0 , GL_TEXTURE_HEIGHT , &height);
        GLubyte* image = new GLubyte[width*height*4];
        if(depth)
        {
            GLfloat* fimg = new GLfloat[width*height];
            glGetTexImage(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT,GL_FLOAT,fimg);
            convertDepthToRGB(fimg,width*height,image);
            delete[] fimg;
        }
        else
        {
            glGetTexImage(GL_TEXTURE_2D,0,GL_RGBA,GL_UNSIGNED_BYTE,image);
        }
        if (!stbi_write_png(filename.c_str(),width,height,4,(void*)image,0))
        {
            cout<<"failed to write image "<<filename<<endl;
            return -1;
        }

        glBindTexture(GL_TEXTURE_2D,0);
        delete[] image;

        return 0;
    }

    void RenderUtils::convertDepthToRGB(const GLfloat* depth, GLuint size, GLubyte* rgba) 
    {
        GLfloat minimum = 1.0f;
        for (GLuint i = 0;i<size;i++)
        {
            if (minimum>depth[i])
            {
                minimum = depth[i];
            }
        }
        GLfloat one_minus_min = 1.f-minimum;
        for (GLuint i = 0;i<size;i++)
        {
            for (GLuint j = 0;j<3;j++)
            {
                rgba[(i*4)+j]=(GLubyte)(((depth[i]-minimum)/one_minus_min) *255U);
            }
            rgba[(i*4)+3] = 255U; //no transparency;
        }
    }

    void RenderUtils::renderBox(float4 min, float4 max, float4 color)
    {

        glEnable(GL_DEPTH_TEST);
        glColor4f(color.x, color.y, color.z, color.w);
        //draw grid
        glBegin(GL_LINES);
        //1st face
        glVertex3f(min.x, min.y, min.z);
        glVertex3f(min.x, min.y, max.z);

        glVertex3f(min.x, max.y, min.z);
        glVertex3f(min.x, max.y, max.z);

        glVertex3f(min.x, min.y, min.z);
        glVertex3f(min.x, max.y, min.z);

        glVertex3f(min.x, min.y, max.z);
        glVertex3f(min.x, max.y, max.z);
        //2nd face
        glVertex3f(max.x, min.y, min.z);
        glVertex3f(max.x, min.y, max.z);

        glVertex3f(max.x, max.y, min.z);
        glVertex3f(max.x, max.y, max.z);

        glVertex3f(max.x, min.y, min.z);
        glVertex3f(max.x, max.y, min.z);

        glVertex3f(max.x, min.y, max.z);
        glVertex3f(max.x, max.y, max.z);
        //connections
        glVertex3f(min.x, min.y, min.z);
        glVertex3f(max.x, min.y, min.z);

        glVertex3f(min.x, max.y, min.z);
        glVertex3f(max.x, max.y, min.z);

        glVertex3f(min.x, min.y, max.z);
        glVertex3f(max.x, min.y, max.z);

        glVertex3f(min.x, max.y, max.z);
        glVertex3f(max.x, max.y, max.z);

        glEnd();
    }

    void RenderUtils::renderQuad(float4 min, float4 max, GLuint tex)
    {

        glColor4f(0.0f, 0.4f, 0.0f, 1.0f);
        glBindTexture(GL_TEXTURE_2D,tex);
        glBegin(GL_QUADS);
        glTexCoord2f(0.f,0.f);
        glVertex3f(min.x, min.y, min.z);
        glTexCoord2f(1.f,0.f);
        glVertex3f(max.x, min.y, min.z);
        glTexCoord2f(1.f,1.f);
        glVertex3f(max.x, max.y, min.z);
        glTexCoord2f(0.f,1.f);
        glVertex3f(min.x, max.y, min.z);
        glEnd();
        glBindTexture(GL_TEXTURE_2D,0);
        //glDisable(GL_DEPTH_TEST);
    }

    GLuint RenderUtils::loadTexture(const string& texFile, const string& texName)
    {
        //Load an image with stb_image
        int w,h,channels;
        int force_channels = 0;

        unsigned char *im = stbi_load( texFile.c_str(), &w, &h, &channels, force_channels );
        if (im == NULL)
        {
            cout<<"Image failed to load\nName: "<<texFile<<"\nReason:"<< stbi_failure_reason()<<endl;
            return 0;
        }

        GLuint retTex=0;
        //load as gl texture
        glGenTextures(1, &retTex);
        glBindTexture(GL_TEXTURE_2D, retTex);

        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        
        //better way to do this?
        if(channels == 3)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, &im[0]);
        }
        else if (channels == 4)
        {
             glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
                  GL_RGBA, GL_UNSIGNED_BYTE, &im[0]);
        }

        glBindTexture(GL_TEXTURE_2D,0);
        free(im);
        return retTex;
    }
}


