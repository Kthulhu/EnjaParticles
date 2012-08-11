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
    void RenderUtils::fullscreenQuad()
    {
        float vertices[] = {-1.0f,-1.0f,
                            1.0f,-1.0f,
                            1.0f,1.0f,
                            -1.0f,1.0f};
        float texCoord[] = {0.0f,0.0f,
                            1.0f,0.0f,
                            1.0f,1.0f,
                            0.0f,1.0f};
        glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,vertices);
        glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,texCoord);

        glDrawArrays(GL_QUADS, 0, 4);

        //glDisableVertexAttribArray(0);
        //glDisableVertexAttribArray(1);
        glPopClientAttrib();
    }
    void RenderUtils::writeTextures(const map<string,GLuint>& texs)
    {
        for (map<string,GLuint>::const_iterator i = texs.begin();i!=texs.end();i++)
        {
            string s(i->first);
            s+=".png";
            //Fixme: This is a hacky way to find out if the texture is Depth.
            //I should probably query GL for info about the texture.
            writeTexture(i->second, s);// , i->first.find("depth"));
        }
    }
    int RenderUtils::writeTexture( GLuint tex, const string& filename)
    {
        glPushAttrib(GL_ENABLE_BIT|GL_TEXTURE_BIT);
        glEnable(GL_TEXTURE_2D);
        //dout<<"Here"<<endl;
        glBindTexture(GL_TEXTURE_2D,tex);
        GLint width=0,height=0,fmt=0;
        glGetTexLevelParameteriv(GL_TEXTURE_2D , 0 , GL_TEXTURE_WIDTH , &width);
        glGetTexLevelParameteriv(GL_TEXTURE_2D , 0 , GL_TEXTURE_HEIGHT , &height);
        glGetTexLevelParameteriv(GL_TEXTURE_2D , 0 , GL_TEXTURE_INTERNAL_FORMAT, &fmt);
        int components=4;
        if(width==0||height==0)
        {
            cout<<"invalid height and width!"<<endl;
            //glDisable(GL_TEXTURE_2D);
            glPopAttrib();
            return -1;
        }
        GLubyte* image = NULL;
        //dout<<"width = "<<width<<"height = "<<height<<endl;
        if(fmt==GL_DEPTH_COMPONENT32||fmt==GL_DEPTH_COMPONENT24 ||fmt==GL_DEPTH_COMPONENT)
        {
            //dout<<"Here"<<endl;
            GLfloat* fimg = new GLfloat[width*height];
            glGetTexImage(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT,GL_FLOAT,fimg);
            image =  new GLubyte[width*height*components];
            convertDepthToRGB(fimg,width*height,image);
            dout<<"Here"<<endl;
            delete[] fimg;
        }
        else if(fmt==GL_RGB32F)
        {
            components=3;
            image =  new GLubyte[width*height*components];
            GLfloat* fimg = new GLfloat[width*height*components];
            glGetTexImage(GL_TEXTURE_2D,0,fmt,GL_FLOAT,image);
            for(int i = 0; i < width*height*components;i++)
            {
                image[i]=(unsigned char)(fimg[i]*255);
            }
            delete[] fimg;
        }
        else
        {
            image =  new GLubyte[width*height*components];
            glGetTexImage(GL_TEXTURE_2D,0,fmt,GL_UNSIGNED_BYTE,image);
        }
        if (!stbi_write_png(filename.c_str(),width,height,components,(void*)image,0))
        {
            cout<<"failed to write image "<<filename<<endl;
            //glDisable(GL_TEXTURE_2D);
            glPopAttrib();
            return -1;
        }
        //dout<<"Here"<<endl;
        //glBindTexture(GL_TEXTURE_2D,0);
        delete[] image;
        //dout<<"Here"<<endl;
        //glDisable(GL_TEXTURE_2D);
        glPopAttrib();
        return 0;
    }


    void RenderUtils::write3DTextureToDisc(GLuint tex,int voxelResolution, const char* filename)
    {
        glPushAttrib(GL_ENABLE_BIT|GL_TEXTURE_BIT);
        glEnable(GL_TEXTURE_3D_EXT);
        printf("writing %s texture to disc.\n",filename);
        glBindTexture(GL_TEXTURE_3D_EXT,tex);
        GLubyte* image = new GLubyte[voxelResolution*voxelResolution*voxelResolution*4];
        //GLubyte* image = new GLubyte[voxelResolution*voxelResolution*voxelResolution*3];
        glGetTexImage(GL_TEXTURE_3D_EXT,0,GL_RGBA,GL_UNSIGNED_BYTE,image);
        //glGetTexImage(GL_TEXTURE_3D_EXT,0,GL_RGB,GL_UNSIGNED_BYTE,image);
        GLubyte* tmp2Dimg = new GLubyte[voxelResolution*voxelResolution*4];
        //GLubyte* tmp2Dimg = new GLubyte[voxelResolution*voxelResolution*3];
        for(int i = 0; i<voxelResolution; i++)
        {
            char fname[128];
            sprintf(fname,"%s-%03d.png",filename,i);
            memcpy(tmp2Dimg,image+(i*(voxelResolution*voxelResolution*4)),sizeof(GLubyte)*voxelResolution*voxelResolution*4);
            //memcpy(tmp2Dimg,image+(i*(voxelResolution*voxelResolution*3)),sizeof(GLubyte)*voxelResolution*voxelResolution*3);
            if (!stbi_write_png(fname,voxelResolution,voxelResolution,4,(void*)tmp2Dimg,0))
            //if (!stbi_write_png(fname,voxelResolution,voxelResolution,3,(void*)tmp2Dimg,0))
            {
                printf("failed to write image %s",filename);
            }
        }
        //glBindTexture(GL_TEXTURE_3D_EXT,0);
        delete[] image;
        glPopAttrib();
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
        //glPushAttrib(GL_ENABLE_BIT);
        //glEnable(GL_DEPTH_TEST);
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
        //glPopAttrib();
    }

    void RenderUtils::renderQuad(float4 min, float4 max, GLuint tex)
    {
        glPushAttrib(GL_ENABLE_BIT|GL_TEXTURE_BIT);
        glEnable(GL_TEXTURE_2D);
        //glColor4f(0.0f, 0.4f, 0.0f, 1.0f);
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
        glPopAttrib();
    }

    GLuint RenderUtils::loadTexture(const string& texFile, const string& texName)
    {
        glPushAttrib(GL_ENABLE_BIT|GL_TEXTURE_BIT);
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
            //glDisable(GL_DEPTH_TEST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, &im[0]);
        }
        else if (channels == 4)
        {
             glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
                  GL_RGBA, GL_UNSIGNED_BYTE, &im[0]);
        }

        //glBindTexture(GL_TEXTURE_2D,0);
        free(im);
        glPopAttrib();
        return retTex;
    }
        GLuint RenderUtils::loadCubemapTexture(const string& texpath)
    {
        glPushAttrib(GL_ENABLE_BIT|GL_TEXTURE_BIT);
        //Load an image with stb_image
        int w=0,h=0,channels=0;
        int force_channels = 0;

        unsigned char *imposx = stbi_load( string(texpath).append("posx.jpg").c_str(), &w, &h, &channels, force_channels );
        unsigned char *imposy = stbi_load( string(texpath).append("posy.jpg").c_str(), &w, &h, &channels, force_channels );
        unsigned char *imposz = stbi_load( string(texpath).append("posz.jpg").c_str(), &w, &h, &channels, force_channels );
        unsigned char *imnegx = stbi_load( string(texpath).append("negx.jpg").c_str(), &w, &h, &channels, force_channels );
        unsigned char *imnegy = stbi_load( string(texpath).append("negy.jpg").c_str(), &w, &h, &channels, force_channels );
        unsigned char *imnegz = stbi_load( string(texpath).append("negz.jpg").c_str(), &w, &h, &channels, force_channels );

		if(channels==0||w==0||h==0)
		{
			cout<<"couldn't read cubemap textures from "<<texpath<<"!"<<endl;
            return -1;
		}
        GLuint retTex=0;
        glEnable(GL_TEXTURE_CUBE_MAP);
        glGenTextures(1, &retTex);
        glBindTexture(GL_TEXTURE_CUBE_MAP, retTex);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_EXT);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_EXT);
        glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_EXT);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        GLenum format = GL_RGBA;
        if(channels = 3)
            format = GL_RGB;
        //Define all 6 faces
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA8, w, h, 0, format, GL_UNSIGNED_BYTE, imposx);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA8, w, h, 0, format, GL_UNSIGNED_BYTE, imnegx);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA8, w, h, 0, format, GL_UNSIGNED_BYTE, imposy);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA8, w, h, 0, format, GL_UNSIGNED_BYTE, imnegy);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA8, w, h, 0, format, GL_UNSIGNED_BYTE, imposz);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA8, w, h, 0, format, GL_UNSIGNED_BYTE, imnegz);
        //glBindTexture(GL_TEXTURE_CUBE_MAP,0);
        //glDisable(GL_TEXTURE_CUBE_MAP);
        glPopAttrib();
        free(imposx);
        free(imposy);
        free(imposz);
        free(imnegx);
        free(imnegy);
        free(imnegz);
        return retTex;
    }
}


