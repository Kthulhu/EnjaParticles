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
#include <GL/glew.h>
#include "MarchingCubes.h"
using namespace std;

namespace rtps
{
    MarchingCubes::MarchingCubes(const MarchingCubes& mc)
    {
        clone(mc);
    }
    const MarchingCubes& MarchingCubes::operator=(const MarchingCubes& mc)
    {
        clone(mc);
        return *this;
    }
    void MarchingCubes::clone(const MarchingCubes& mc)
    {
        origin[0]=mc.origin[0];origin[1]=mc.origin[2];origin[2]=mc.origin[2];
        region[0]=mc.region[0];region[1]=mc.region[1];region[2]=mc.region[2];
        k_classify=mc.k_classify;
        k_construct=mc.k_classify;
        //k_traverse.resize(mc.k_traverse.size());
        //copy(mc.k_traverse.begin(),mc.k_traverse.end(),k_traverse.begin());
        k_traverse=mc.k_traverse;
        cl_triangles=mc.cl_triangles;
        cl_normals=mc.cl_normals;
        //cl_histopyramid.resize(mc.cl_histopyramid.size());
        //copy(mc.cl_histopyramid.begin(),mc.cl_histopyramid.end(),cl_histopyramid.begin());
        cl_histopyramid=mc.cl_histopyramid;
        cli = mc.cli;
        timer = mc.timer;
        res = mc.res;
        texRes2D=mc.texRes2D;
        slices=mc.slices;
        levels=mc.levels;
        mesh=mc.mesh;
    }
    //----------------------------------------------------------------------
    MarchingCubes::MarchingCubes(std::string path, CL* cli_, EB::Timer* timer_,unsigned int res)
    {
        cli = cli_;
        timer = timer_;
        if(res<2)
            this->res=2;
        else
            this->res = res;
        origin[0]=0;origin[1]=0;origin[2]=0;
        region[0]=2;region[1]=2;region[2]=1;
        try
        {
            path = path + "/marchingcubes.cl";
            k_classify = Kernel(cli, path, "classifyCubes2D");
            k_construct = Kernel(cli, path, "constructHPLevel2D");
            //Really lame that opencl doesn't allow variable number of arguments.
            //Opencl 1.2 allows for 2D image arrays. But for current compatability
            //I use diff kernels for each of the different levels.
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
        slices = 1u<<static_cast<unsigned int>(ceil(log(ceil(sqrt(res))/log(2))));
        texRes2D = res*slices;
        levels= ceil(log(texRes2D)/log(2));
        if(mesh.vbo)
        {
            glDeleteBuffers(1, (GLuint*)&mesh.vbo);
            glDeleteBuffers(1, (GLuint*)&mesh.normalbo);
        }
        mesh.vbo=0;
        mesh.vboSize=0;
        mesh.normalbo=0;
        mesh.ibo = 0;
        mesh.iboSize=0;
        cl_histopyramid.resize(levels);
        unsigned int levelRes = texRes2D;
        //dout<<"level = "<<0<<" levelRes = "<<levelRes<<endl;
        /*float* zeroImg = new float[texRes2D*texRes2D*4];
        memset(zeroImg,0,texRes2D*texRes2D*4*sizeof(float));
        cl_histopyramid[0]=cl::Image2D(cli->context,CL_MEM_READ_WRITE,cl::ImageFormat(CL_RGBA, CL_FLOAT),texRes2D,texRes2D,0,zeroImg);
        delete[] zeroImg;*/
        for(float i = 0; i<levels; i++)
        {
            float* zeroImg2 = new float[levelRes*levelRes*4];
            memset(zeroImg2,0,levelRes*levelRes*4*sizeof(float));
            //dout<<"level = "<<i<<" levelRes = "<<levelRes<<endl;
            cl_histopyramid[i]=cl::Image2D(cli->context,CL_MEM_READ_WRITE,cl::ImageFormat(CL_RGBA, CL_FLOAT),levelRes,levelRes,0,zeroImg2);
            delete[] zeroImg2;
            levelRes /=2;
        }
    }

    struct Mesh* MarchingCubes::execute(cl::Image2D& colorfield,
                    unsigned int res,
                    //debug params
                    Buffer<float4>& clf_debug,
                    Buffer<int4>& cli_debug)
    {

        int iarg = 0;

        //dout<<"Here"<<endl;
        if(res!=this->res)
        {
            if(res<2)
                this->res=2;
            else
                this->res = res;
            //dout<<"levels = "<<levels<<" res = "<<res<<" slices = "<<slices<<endl;
            initializeData();
            //dout<<"levels = "<<levels<<" res = "<<res<<" slices = "<<slices<<endl;
        }

        //dout<<"levels = "<<levels<<" res = "<<res<<" slices = "<<slices<<endl;
        k_classify.setArg(iarg++,cl_histopyramid[0]);
        k_classify.setArg(iarg++,colorfield);
        k_classify.setArg(iarg++,res);
        k_classify.setArg(iarg++,slices);
        k_classify.setArg(iarg++,0.0f);

        try
        {
            float gputime = k_classify.execute(cl::NDRange(res,res,res));
            if(gputime > 0)
                timer->set(gputime);
        }
        catch (cl::Error er)
        {
            printf("ERROR(marchingcubes ): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
        }

#if 1
        //DEBUGGING!!
        try
        {
            float* tmpImg = new float[texRes2D*texRes2D*4];
            memset(tmpImg,0,texRes2D*texRes2D*4*sizeof(float));
            cl::size_t<3> tmpregion;
            tmpregion[0]=texRes2D;
            tmpregion[1]=texRes2D;
            tmpregion[2]=1;
            cli->queue.enqueueReadImage(cl_histopyramid[0], CL_TRUE, origin, tmpregion, 0, 0, tmpImg);
            dout<<"Here"<<endl;
            cli->queue.finish();
            float t=0;
            for(int j = 0;j<texRes2D*texRes2D*4;j+=4)
            {
                if(tmpImg[j]>5)
                    dout<<"WTF Shouldn't be greater than 5."<<endl;
                t+=tmpImg[j];
            }
            delete[] tmpImg;
            dout<<"Total triangles = "<<t<<endl;
        }
        catch (cl::Error er)
        {
            printf("ERROR(marchingcubes ): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
        }
#endif
        //dout<<"Here"<<endl;

        unsigned int levelRes = texRes2D;
        for(unsigned int i = 1; i<levels; i++)
        {
            levelRes/=2;
            iarg=0;
            k_construct.setArg(iarg++,cl_histopyramid[i]);
            k_construct.setArg(iarg++,cl_histopyramid[i-1]);
            //k_construct.setArg(iarg++,clf_debug.getDevicePtr());
            try
            {
dout<<"levelres = "<<levelRes<<endl;
                float gputime = k_construct.execute(cl::NDRange(levelRes,levelRes));
                if(gputime > 0)
                    timer->set(gputime);
            }
            catch (cl::Error er)
            {
                printf("ERROR(marchingcubes ): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
            }

#if 0
        //DEBUGING

        int num =levelRes*levelRes;
        if(num > 0)// && choice == 0)
        {
            printf("============================================\n");
            printf("***** PRINT colorfield diagnostics ******\n");
            printf("num %d\n", num);

            std::vector<float4> clf(num);

            clf_debug.copyToHost(clf);

            for (int i=0; i < num; i++)
            {
                if(clf[i].x)
                {
                    printf("clf_debug: %f, %f, %f, %f\n", clf[i].x, clf[i].y, clf[i].z, clf[i].w);
                }
            }
        }

        //DEBUGGING!!
        try
        {
            float* tmpImg = new float[levelRes*levelRes*4];
            memset(tmpImg,0,levelRes*levelRes*4*sizeof(float));
            cl::size_t<3> tmpregion;
            tmpregion[0]=levelRes;
            tmpregion[1]=levelRes;
            tmpregion[2]=1;
            cli->queue.enqueueReadImage(cl_histopyramid[i], CL_TRUE, origin, tmpregion, 0, 0, tmpImg);
            dout<<"Here"<<endl;
            cli->queue.finish();
            float t=0;
            for(int j = 0;j<levelRes*levelRes*4;j+=4)
            {
                t+=tmpImg[j];
            }
            delete[] tmpImg;
            dout<<"Total triangles = "<<t<<endl;
        }
        catch (cl::Error er)
        {
            printf("ERROR(marchingcubes ): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
        }
#endif

        }

        float totals[16]={0.0f};
        unsigned int total=0;
        try
        {
            cli->queue.enqueueReadImage(cl_histopyramid[levels-1], CL_FALSE, origin, region, 0, 0, totals);
            dout<<"Here"<<endl;
            cli->queue.finish();
            total=(unsigned int)(totals[0]+totals[4]+totals[8]+totals[12]);
            dout<<"Total triangles = "<<total<<endl;
        }
        catch (cl::Error er)
        {
            printf("ERROR(marchingcubes ): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
        }
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
            cl_triangles=Buffer<float>(cli,mesh.vbo);
            cl_normals=Buffer<float>(cli,mesh.normalbo);
            glFinish();
        }
        mesh.vboSize=total;
        if(total!=0)
        {
            iarg=0;
            cl_triangles.acquire();
            cl_normals.acquire();
            for(int i = 0; i<levels; i++)
                k_traverse[levels-1].setArg(iarg++,cl_histopyramid[i]);
            k_traverse[levels-1].setArg(iarg++,cl_triangles);
            k_traverse[levels-1].setArg(iarg++,cl_normals);
            k_traverse[levels-1].setArg(iarg++,res);
            k_traverse[levels-1].setArg(iarg++,slices);
            k_traverse[levels-1].setArg(iarg++,0.0f);
            k_traverse[levels-1].setArg(iarg++,total);
            try
            {
                float gputime = k_traverse[levels-1].execute(cl::NDRange(total));
                if(gputime > 0)
                    timer->set(gputime);
            }
            catch (cl::Error er)
            {
                printf("ERROR(marchingcubes ): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
            }

            dout<<"Here"<<endl;
            cl_triangles.release();
            cl_normals.release();
        }
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
        return &mesh;
    }


}

