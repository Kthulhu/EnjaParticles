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
#include <stdlib.h>
#include <algorithm>
#include <vector>

#include <GL/glew.h>

#include "StreamlineEffect.h"
#include "Shader.h"
#include "util.h"
#include "ShaderLibrary.h"

using namespace std;

namespace rtps
{

    StreamlineEffect::StreamlineEffect(RenderSettings rs, ShaderLibrary& lib, unsigned int maxLength, unsigned int num, vector<unsigned int>& indices, CL* cli):ParticleEffect(rs,lib)
    {
        m_maxSLLength=maxLength;
        m_numSL = num;
        m_curSLIndex = 1;
        vector<float4> f4vec((m_maxSLLength+1)*m_numSL);
        std::fill(f4vec.begin(), f4vec.end(), float4(0.0f, 0.0f, 0.0f, 0.0f));
        m_streamLineCP = createVBO(&f4vec[0], f4vec.size()*sizeof(float4), GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
        m_streamLineColor = createVBO(&f4vec[0], f4vec.size()*sizeof(float4), GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW);
        m_clStreamLineCP=Buffer<float4>(cli,m_streamLineCP);
        m_clStreamLineColor=Buffer<float4>(cli,m_streamLineColor);
        m_clSampleIndices=Buffer<unsigned int>(cli,indices);

        sample = Sample("./bin/"+std::string(COMMON_CL_SOURCE_DIR), cli);
    }



    //----------------------------------------------------------------------
    StreamlineEffect::~StreamlineEffect()
    {
        if(m_streamLineCP)
            glDeleteBuffers(1, &m_streamLineCP);
        if(m_streamLineColor)
            glDeleteBuffers(1, &m_streamLineColor);
    }

    void StreamlineEffect::render()
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_streamLineCP);
        glVertexPointer(4, GL_FLOAT, 0, (char *) NULL);
        glBindBuffer(GL_ARRAY_BUFFER, m_streamLineColor);
        glColorPointer(4, GL_FLOAT, 0, (char *) NULL);

        if(m_curSLIndex>2)
        {
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_COLOR_ARRAY);
            for(unsigned int i = 0; i<m_numSL; i++)
                glDrawArrays(GL_LINE_STRIP, (i*m_maxSLLength)+1, m_curSLIndex);
            //glDrawArrays(GL_LINE_STRIP, 0, m_curSLIndex);
            glDisableClientState(GL_COLOR_ARRAY);
            glDisableClientState(GL_VERTEX_ARRAY);
        }
        glBindBuffer(GL_ARRAY_BUFFER,0);
        //make sure rendering timing is accurate
        glFinish();
    }

    void StreamlineEffect::addStreamLine(Buffer<float4>& pos, Buffer<float4>& col, unsigned int num)
    {
        if(num>0)
        {
            m_clStreamLineCP.acquire();
            m_clStreamLineColor.acquire();
            sample.execute(num,pos,m_maxSLLength,m_clStreamLineCP,m_clSampleIndices, m_curSLIndex,m_maxSLLength);
            sample.execute(num,col,m_maxSLLength,m_clStreamLineColor,m_clSampleIndices, m_curSLIndex,m_maxSLLength);
            m_clStreamLineCP.release();
            m_clStreamLineColor.release();
            if(m_curSLIndex==m_maxSLLength)
                m_curSLIndex = 0;
            else
                m_curSLIndex++;
        }
    }
}


