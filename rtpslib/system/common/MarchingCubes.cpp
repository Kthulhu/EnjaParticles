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

#include <math.h>
#include "MarchingCubes.h"

namespace rtps
{
    size_t<3> origin;
    origin[0]=0;origin[1]=0;origin[2]=0;
    size_t<3> region;
    region[0]=2;region[1]=2;region[2]=1;
    //----------------------------------------------------------------------
    MarchingCubes::MarchingCubes(std::string path, CL* cli_, EB::Timer* timer_,unsigned int res)
    {
        cli = cli_;
        timer = timer_;
        if(res<2)
            this->res=2;
        else
            this->res = res;
        levels= ceil(log(res)/log(2));
        slices = ceil(sqrt(res));
        texRes2D = res*ceil(sqrt(res));

        try
        {
            path = path + "/marchingcubes.cl";
            k_classify = Kernel(cli, path, "classifyCubes2D");
            k_construct = Kernel(cli, path, "constructHPLevel2D");
            k_traverse.push_back(Kernel(cli, path, "traverseHP2D1"));
            k_traverse.push_back(Kernel(cli, path, "traverseHP2D2"));
            k_traverse.push_back(Kernel(cli, path, "traverseHP2D3"));
            k_traverse.push_back(Kernel(cli, path, "traverseHP2D4"));
            k_traverse.push_back(Kernel(cli, path, "traverseHP2D5"));
            k_traverse.push_back(Kernel(cli, path, "traverseHP2D6"));
            k_traverse.push_back(Kernel(cli, path, "traverseHP2D7"));
            k_traverse.push_back(Kernel(cli, path, "traverseHP2D8"));
            k_traverse.push_back(Kernel(cli, path, "traverseHP2D9"));
            k_traverse.push_back(Kernel(cli, path, "traverseHP2D10"));
        }
        catch (cl::Error er)
        {
            printf("ERROR(MarchingCubes): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
        }
        initializeData();
    }
    //----------------------------------------------------------------------

    void MarchingCubes::initializeData()
    {

        mesh.vbo=0;
        mesh.vboSize=0;
        mesh.normalbo=0;
        mesh.ibo = 0;
        mesh.iboSize=0;
        cl_histopyramid.resize(levels);
        cl_histopyramid[0]=cl::Image2D(cli->context,CL_MEM_READ_WRITE,ImageFormat(CL_RGBA, CL_UNSIGNED_INT8),texRes,texRes);
        for(unsigned int i = 1; i<levels; i++)
        {
            cl_histopyramid[i]=cl::Image2D(cli->context,CL_MEM_READ_WRITE,ImageFormat(CL_R, CL_UNSIGNED_INT32),texRes,texRes);
        }
    }

    struct Mesh& MarchingCubes::execute(cl::Image2D& colorfield,
                    unsigned int res,
                    //debug params
                    Buffer<float4>& clf_debug,
                    Buffer<int4>& cli_debug)
    {
        int iarg = 0;

        if(res!=this->res)
        {
            this->res = res;
            levels = ceil(log(res)/log(2));
            slices = ceil(sqrt(res));
            texRes2D = res*slices;
            initalizeData();
        }

        k_classify.setArg(iarg++,cl_histopyramid[0]);
        k_classify.setArg(iarg++,colorfield);
        k_classify.setArg(iarg++,slices);
        k_classify.setArg(iarg++,res);
        k_classify.setArg(iarg++,0);

        int local = 64;
        try
        {
            float gputime = k_classify.execute(cl::NDRange(res,res,res));
            if(gputime > 0)
                timer->set(gputime);
        }
        catch (cl::Error er)
        {
            printf("ERROR(force ): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
        }


        iarg=0;
        for(unsigned int i = 0; i<levels; i++)
        {
            k_construct.setArg(iarg++,cl_histopyramid[i]);
            k_construct.setArg(iarg++,cl_histopyramid[i+1]);
            k_construct.setArg(iarg++,slices);
            k_construct.setArg(iarg++,res);
            int local = 64;
            try
            {
                float gputime = k_classify.execute(cl::NDRange(res,res,res));
                if(gputime > 0)
                    timer->set(gputime);

            }
            catch (cl::Error er)
            {
                printf("ERROR(force ): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
            }
        }

        //TODO read total back to cpu
        unsigned int totals[4];
        cli->queue.enqueueReadImage(cl_histopyramid[levels-1], CL_FALSE, origin, region, 0, 0, totals);
        unsigned int total=totals[0]+totals[1]+totals[2]+totals[3];
        if(total>mesh.vboSize)
        {
            if(mesh.vbo)
            {
                glDeleteBuffers(1, (GLuint*)&mesh.vbo);
                glDeleteBuffers(1, (GLuint*)&mesh.normalbo);
            }
            glGenBuffers(1,&mesh.vbo);
            glBindBuffer(GL_ARRAY_BUFFER,mesh.vbo);
            glBufferData(GL_ARRAY_BUFFER,total*9*sizeof(float),NULL, GL_DYNAMIC_DRAW);
            glGenBuffers(1,&mesh.normalbo);
            glBindBuffer(GL_ARRAY_BUFFER,mesh.normalbo);
            glBufferData(GL_ARRAY_BUFFER,total*9*sizeof(float),NULL, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER,0);
            mesh.vboSize=total;
            cl_triangles=Buffer<float>(cli,mesh.vbo);
            cl_normals=Buffer<float>(cli,mesh.normalbo);
            glFinish();
        }
        iarg=0;
        cl_triangles.acquire();
        cl_normals.acquire();
        for(int i = 0; i<levels; i++)
            k_traverse[levels].setArg(iarg++,cl_histopyramid[i]);
        k_traverse[levels].setArg(iarg++,cl_triangles);
        k_traverse[levels].setArg(iarg++,cl_normals);
        k_traverse[levels].setArg(iarg++,slices);
        k_traverse[levels].setArg(iarg++,res);
        k_traverse[levels].setArg(iarg++,0);
        k_traverse[levels].setArg(iarg++,total);
        int local = 64;
        try
        {
            float gputime = k_classify.execute(cl::NDRange(res,res,res));
            if(gputime > 0)
                timer->set(gputime);
        }
        catch (cl::Error er)
        {
            printf("ERROR(force ): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
        }

        cl_triangles.release();
        cl_normals.release();

#if 0 //printouts
        //DEBUGING

        int num = res*res;
        if(num > 0)// && choice == 0)
        {
            printf("============================================\n");
            printf("***** PRINT marchingcubes diagnostics ******\n");
            printf("num %d\n", num);

            std::vector<int4> cli(num);
            std::vector<float4> clf(num);

            cli_debug.copyToHost(cli);
            clf_debug.copyToHost(clf);

            std::vector<float4> poss(num);
            std::vector<float4> dens(num);

            for (int i=0; i < num; i++)
            //for (int i=0; i < 10; i++)
            {
                //printf("-----\n");
                printf("clf_debug: %f, %f, %f, %f\n", clf[i].x, clf[i].y, clf[i].z, clf[i].w);
                //if(clf[i].w == 0.0) exit(0);
                //printf("cli_debug: %d, %d, %d, %d\n", cli[i].x, cli[i].y, cli[i].z, cli[i].w);
                //		printf("pos : %f, %f, %f, %f\n", pos[i].x, pos[i].y, pos[i].z, pos[i].w);
            }
        }
#endif
        return mesh;
    }


}

