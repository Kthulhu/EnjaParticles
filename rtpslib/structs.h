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


#ifndef RTPS_STRUCTS_H_INCLUDED
#define RTPS_STRUCTS_H_INCLUDED
#include "rtps_common.h"
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <boost/tokenizer.hpp>
#include "debug.h"


namespace rtps
{

    typedef struct float3
    {
        //we have to add 4th component to match how OpenCL does float3 on GPU
        float x, y, z, w;
        float3()
        {
		x=0.0f; y=0.0f; z=0.0f; w=0.0f;
        }
        float3(float x, float y, float z)
        {
            this->x = x;
            this->y = y;
            this->z = z;
	    this->w = 0.0f;
        }
        friend float3 operator+(float3& a, float3& b)
        {
            float3 c = float3(a.x+b.x, a.y+b.y, a.z+b.z);
            return c;
        }
        friend float3 operator-(float3& a, float3& b)
        {
            float3 c = float3(a.x-b.x, a.y-b.y, a.z-b.z);
            return c;
        }
        void print(const char* msg=0)
        {
            printf("%s: %f, %f, %f\n", msg, x, y, z);
        }

        float3& operator*(float b)
        {
            x*=b;
            y*=b;
            z*=b;
            return *this;
        }
    } float3;

    typedef struct int2
    {
        int x, y;
        int2()
        {
            x=0;
            y=0;
        }
		int2(int x, int y)
        {
            this->x = x;
            this->y = y;
        }
        int2(float x, float y)
        {
            this->x = (int)x;
            this->y = (int)y;
        }
    } int2;
    // GE: Sept. 8, 2010
    typedef struct int3
    {
        int x, y, z, w;
        int3()
        {
            x=0;y=0;z=0;w=0;
        }
	int3(int x, int y, int z)
        {
            this->x = x;
            this->y = y;
            this->z = z;
            this->w = 0;
        }
        int3(float x, float y, float z)
        {
            this->x = (int)x;
            this->y = (int)y;
            this->z = (int)z;
            this->w = 0;
        }
    } int3;

    // GE: Sept. 8, 2010
    // Coded as int4 since OpenCL does not have int3
    typedef struct int4
    {
        int x, y, z, w;
        int4()
        {
            x=0;y=0;z=0;w=0;
        }
        int4(float x, float y, float z, float w=1.)
        {
            this->x = (int)x;
            this->y = (int)y;
            this->z = (int)z;
            this->w = (int)w;
        }
        int4(int x, int y, int z, int w=1)
        {
            this->x = x;
            this->y = y;
            this->z = z;
            this->w = w;
        }
        void print(const char* msg=0)
        {
            printf("%s: %d, %d, %d, %d\n", msg, x, y, z, w);
        }
    } int4;

    //temporary
    typedef struct float16
    {
        float m[16];

        float16()
        {
            m[0]= 0; m[1]= 0; m[2]= 0; m[3]= 0;
            m[4]= 0; m[5]= 0; m[6]= 0; m[7]= 0;
            m[8]= 0; m[9]= 0; m[10]= 0; m[11]= 0;
            m[12]= 0; m[13]= 0; m[14]= 0; m[15]= 0;
        }
        float16(float a, float b, float c, float d,
                float e, float f, float g, float h,
                float i, float j, float k, float l,
                float n, float o, float p, float q)
        {
            m[0]= a; m[1]= b; m[2]= c; m[3]= d;
            m[4]= e; m[5]= f; m[6]= g; m[7]= h;
            m[8]= i; m[9]= j; m[10]= k; m[11]= l;
            m[12]= n; m[13]= o; m[14]= p; m[15]= q;
        }
        void transpose()
        {
            for(int i = 0; i<4; i++)
            {
                for(int j = i+1; j<4; j++)
                {
                    int index = i*4+j;
                    int index2 = j*4+i;
                    float tmp = m[index];
                    m[index]=m[index2];
                    m[index2]=tmp;
                }
            }
        }
        friend float16 operator*(const float16& lhs,const float16& rhs)
        {
            float16 retval;
            for(int i = 0; i<16;i++)
            {
                for(int j = 0; j<4; j++)
                {
                    retval.m[i]+=lhs.m[(i/4)*4+j]*rhs.m[(i%4)+j*4];
                }
            }
            return retval;
        }

        void print(const char* msg=0)
        {
            printf("%s: %f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n",
                    msg, m[0],m[1],m[2],m[3],
                    m[4],m[5],m[6],m[7],
                    m[8],m[9],m[10],m[11],
                    m[12],m[13],m[14],m[15]);
        }
    } float16;
#ifdef WIN32
#pragma pack(push,16)
#endif

    // IJ
    typedef struct float4
    {
        float x;
        float y;
        float z;
        float w;

        float4()
        {
            x=0.0f;
            y=0.0f;
            z=0.0f;
            w=0.0f;
        };
        float4(const float3& f, float ww):
        x(f.x),
        y(f.y),
        z(f.z),
        w(ww)
        {}
        float4(float xx, float yy, float zz, float ww):
        x(xx),
        y(yy),
        z(zz),
        w(ww)
        {
        }
        void set(float xx, float yy, float zz, float ww=1.)
        {
            x = xx;
            y = yy;
            z = zz;
            w = ww;
        }

        void print(const char* msg=0)
        {
            printf("%s: %e, %e, %e, %f\n", msg, x, y, z, w);
        }

        void printd(const char* msg=0) {
            printf("%s: %18.11e, %18.11e, %18.11e, %f\n", msg, x, y, z, w);
        }
                friend float4 operator-(float4& a, float4& b)
        {
            float4 c = float4(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);
            return c;
        }

        // to do: float4 aa = min - float4(5.,5.,5.,5.); // min is float4
        friend const float4 operator-(const float4& a, const float4& b)
        {
            float4 c = float4(a.x-b.x, a.y-b.y, a.z-b.z, a.w-b.w);
            return c;
        }

        friend float4 operator-(float4&a)
        {
            float4 c = float4(-a.x, -a.y, -a.z, -a.w);
            return c;
        }

        friend float4 operator+(float4& a, float4& b)
        {
            float4 c = float4(b.x+a.x, b.y+a.y, b.z+a.z, b.w+a.w);
            return c;
        }

        friend const float4 operator+(const float4& a, const float4& b)
        {
            float4 c = float4(b.x+a.x, b.y+a.y, b.z+a.z, b.w+a.w);
            return c;
        }

        void operator+=(float4 a)
        {
            (*this).x += a.x;
            (*this).y += a.y;
            (*this).z += a.z;
            (*this).w += a.w;
        }

        void operator-=(float4 a)
        {
            (*this).x -= a.x;
            (*this).y -= a.y;
            (*this).z -= a.z;
            (*this).w -= a.w;
        }

        void operator*=(float4 a)
        {
            (*this).x *= a.x;
            (*this).y *= a.y;
            (*this).z *= a.z;
            (*this).w *= a.w;
        }
        void operator*=(float a)
        {
            (*this).x *= a;
            (*this).y *= a;
            (*this).z *= a;
            (*this).w *= a;
        }

        void operator/=(float a)
        {
            (*this).x /= a;
            (*this).y /= a;
            (*this).z /= a;
            (*this).w /= a;
        }
        void operator/=(float4 a)
        {
            (*this).x /= a.x;
            (*this).y /= a.y;
            (*this).z /= a.z;
            (*this).w /= a.w;
        }

        friend float4 operator*(float r, float4& b)
        {
            float4 m = float4(r*b.x, r*b.y, r*b.z, r*b.w);
            return m;
        }
        friend float4 operator*(float4& b, float r)
        {
            float4 m = float4(r*b.x, r*b.y, r*b.z, r*b.w);
            return m;
        }

        friend float4 operator/(float4& b, float r)
        {
            float d = 1.f/r;
            float4 m = float4(d*b.x, d*b.y, d*b.z, d*b.w);
            return m;
        }
        friend float4 operator*(const float16& mat, float4& b)
        {
            float4 retval(0.0f,0.0f,0.0f,0.0f);
            retval.x=mat.m[0]*b.x+mat.m[1]*b.y+mat.m[2]*b.z+mat.m[3]*b.w;
            retval.y=mat.m[4]*b.x+mat.m[5]*b.y+mat.m[6]*b.z+mat.m[7]*b.w;
            retval.z=mat.m[8]*b.x+mat.m[9]*b.y+mat.m[10]*b.z+mat.m[11]*b.w;
            retval.w=mat.m[12]*b.x+mat.m[13]*b.y+mat.m[14]*b.z+mat.m[15]*b.w;
            return retval;
        }
        friend std::istream& operator>>(std::istream& is, float4& __n)
        {
                is>>__n.x>>__n.y>>__n.z>>__n.w;
                return is;
        }
        friend std::ostream& operator<<(std::ostream& os, float4& __n)
        {
                os<<__n.x<<" "<<__n.y<<" "<<__n.z<<" "<<__n.w;
                return os;
        }

        /*float4& operator/(float r)
        {
            x/=r;
            y/=r;
            z/=r;
            w/=r;
            return *this;
        }*/

        float length()
        {
            float4& f = *this;
            return sqrt(f.x*f.x + f.y*f.y + f.z*f.z);
        }


    } float4
#ifndef WIN32
	__attribute__((aligned(16)));
#else
		;
        #pragma pack(pop)
#endif

    // size: 4*4 = 16 floats
    // shared memory = 65,536 bytes = 16,384 floats
    //               = 1024 triangles
    typedef struct Triangle
    {
        float4 verts[3];
        float4 normal;    //should pack this in verts array
    } Triangle;

    //Helper Read functions for Parameters.
    template<typename scalar>
    std::istream& operator>>(std::istream& is, std::vector<scalar>& __n)
    {
        scalar s;
        while(is.good())
        {
            is>>s;
            __n.push_back(s);
        }

        return is;
    }

    /*std::istream& operator>>(std::istream& is, std::vector<float>& __n)
    {
        float f;
        while(is.good())
        {
            is>>f;
            __n.push_back(f);
        }
        for(int i=0;i<__n.size();i++)
        {
            dout<<"float "<<i<<" "<<__n[i]<<std::endl;
        }
        return is;
    }*/
    //maybe these helper functions should go elsewhere?
    //or be functions of the structs
    RTPS_EXPORT float magnitude(float4 vec);
    RTPS_EXPORT float dist_squared(float4 vec);
    RTPS_EXPORT float dot(float4 a, float4 b);
    RTPS_EXPORT float4 cross(float4 a, float4 b);
    RTPS_EXPORT float4 normalize(float4 vect);
	RTPS_EXPORT float4 normalize3(float4 vect); // only use first 3 components of vect
	RTPS_EXPORT float magnitude3(float4 vec); // only use first 3 components of vec

}

#endif
