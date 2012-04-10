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
#include <sstream>
#include <GL/glew.h>
#include "MarchingCubes.h"

using namespace std;

namespace rtps
{
    MarchingCubes::MarchingCubes(const MarchingCubes& mc)
    {
        clone(mc);
    }
    MarchingCubes& MarchingCubes::operator=(const MarchingCubes& mc)
    {
        clone(mc);
        return *this;
    }
    void MarchingCubes::clone(const MarchingCubes& mc)
    {
        origin[0]=mc.origin[0];origin[1]=mc.origin[2];origin[2]=mc.origin[2];
        region[0]=mc.region[0];region[1]=mc.region[1];region[2]=mc.region[2];
        k_classify=mc.k_classify;
        k_construct=mc.k_construct;
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
            for(unsigned int i = 0; i<15;i++)
            {
                stringstream s;
                s<<"traverseHP2D"<<i+1;
                k_traverse.push_back(Kernel(cli,path,s.str()));
                //k_traverse.push_back(Kernel(cli,path,"traverseHP2D"));
                dout<<"Num_args = "<<k_traverse[i].kernel.getInfo<CL_KERNEL_NUM_ARGS>()<<endl;
            }
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
        slices = 1u<<static_cast<unsigned int>(ceil(log(ceil(sqrt((float)res))/log(2.0f))));
        texRes2D = res*slices;
        levels= ceil(log((float)texRes2D)/log(2.0f));
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
        mesh.material.ambient=float3(0.0f,0.2f,0.6f);
        mesh.material.diffuse=float3(0.0f,0.2f,0.6f);
        mesh.material.specular=float3(1.0f,1.f,1.0f);
        mesh.material.opacity=0.4;
        mesh.material.shininess=50;
        cl_histopyramid.resize(levels);
        unsigned int levelRes = texRes2D;
        //dout<<"level = "<<0<<" levelRes = "<<levelRes<<endl;
        float* zeroImg = new float[texRes2D*texRes2D*4];
        memset(zeroImg,0,texRes2D*texRes2D*4*sizeof(float));
        cl_histopyramid[0]=cl::Image2D(cli->context,CL_MEM_READ_WRITE,cl::ImageFormat(CL_RGBA, CL_FLOAT),texRes2D,texRes2D,0,zeroImg);
        cli->queue.finish();
        delete[] zeroImg;
        for(float i = 1; i<levels; i++)
        {
#if 0
            float* zeroImg2 = new float[levelRes*levelRes*4];
            memset(zeroImg2,0,levelRes*levelRes*4*sizeof(float));
            //dout<<"level = "<<i<<" levelRes = "<<levelRes<<endl;
            cl_histopyramid[i]=cl::Image2D(cli->context,CL_MEM_READ_WRITE,cl::ImageFormat(CL_RGBA, CL_FLOAT),levelRes,levelRes,0,zeroImg2);
            delete[] zeroImg2;
#endif
            levelRes /=2;
            float* zeroImg2 = new float[levelRes*levelRes];
            memset(zeroImg2,0,levelRes*levelRes*sizeof(float));
            //dout<<"level = "<<i<<" levelRes = "<<levelRes<<endl;
            cl_histopyramid[i]=cl::Image2D(cli->context,CL_MEM_READ_WRITE,cl::ImageFormat(CL_R, CL_FLOAT),levelRes,levelRes,0,zeroImg2);
            cli->queue.finish();
            delete[] zeroImg2;

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
	float isolevel = 0.0001f;

        //dout<<"levels = "<<levels<<" res = "<<res<<" slices = "<<slices<<endl;
        k_classify.setArg(iarg++,cl_histopyramid[0]);
        k_classify.setArg(iarg++,colorfield);
        k_classify.setArg(iarg++,res);
        k_classify.setArg(iarg++,slices);
        k_classify.setArg(iarg++,isolevel);

        try
        {
            float gputime = k_classify.execute(cl::NDRange(res-1,res-1,res-1));
            if(gputime > 0)
                timer->set(gputime);
        }
        catch (cl::Error er)
        {
            printf("ERROR(marchingcubes ): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
        }

#if 0
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
            cli->queue.finish();
            dout<<"Marching cubes -----------------"<<endl;
            float t=0;
            for(int j = 0;j<texRes2D*texRes2D*4;j+=4)
            {
                //if(tmpImg[j]>5)
                //    dout<<"WTF Shouldn't be greater than 5."<<endl;
                //t+=tmpImg[j];
                //if(tmpImg[j+1]<0.0f || tmpImg[j+1]>256.0f)
                //    dout<<"index out of range!!!"<<endl;
                dout<<tmpImg[j]<<","<<tmpImg[j+1]<<","<<tmpImg[j+2]<<","<<tmpImg[j+3]<<endl;
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
            //dout<<"i "<<i<<" texRes2D "<<texRes2D<<" levelRes = "<<levelRes<<endl;
            iarg=0;
            k_construct.setArg(iarg++,cl_histopyramid[i]);
            k_construct.setArg(iarg++,cl_histopyramid[i-1]);
            //k_construct.setArg(iarg++,clf_debug.getDevicePtr());
            try
            {
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
            float t3=0.0f;
            for(int j = 0;j<num;j++)
            {
                t3+=clf[j].x;
            }
            dout<<"Total tris = "<<t3<<endl;

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

        //float totals[16]={0.0f};
        float totals[4]={0.0f};
        unsigned int total=0;
        try
        {
            cli->queue.enqueueReadImage(cl_histopyramid[levels-1], CL_FALSE, origin, region, 0, 0, totals);
            //dout<<"Here"<<endl;
            cli->queue.finish();
            //total=(unsigned int)(totals[0]+totals[4]+totals[8]+totals[12]);
            total=(unsigned int)(totals[0]+totals[1]+totals[2]+totals[3]);
            dout<<"Total triangles = "<<total<<endl;
        }
        catch (cl::Error er)
        {
            printf("ERROR(marchingcubes ): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
        }
        if(total*3>mesh.vboSize)
        {
            if(mesh.vbo)
            {
                glDeleteBuffers(1, (GLuint*)&mesh.vbo);
                glDeleteBuffers(1, (GLuint*)&mesh.normalbo);
            }
            glGenBuffers(1,&mesh.vbo);
            glBindBuffer(GL_ARRAY_BUFFER,mesh.vbo);
            glBufferData(GL_ARRAY_BUFFER,total*3*3*sizeof(float),NULL, GL_DYNAMIC_DRAW);
            glGenBuffers(1,&mesh.normalbo);
            glBindBuffer(GL_ARRAY_BUFFER,mesh.normalbo);
            glBufferData(GL_ARRAY_BUFFER,total*3*3*sizeof(float),NULL, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER,0);
#if 1
            cl_triangles=Buffer<float>(cli,mesh.vbo);
            cl_normals=Buffer<float>(cli,mesh.normalbo);
#else
			//fordebugging
			vector<float> tmp(total*3*3);
			fill(tmp.begin(),tmp.end(),0.0f);
			cl_triangles=Buffer<float>(cli,tmp);
			cl_normals=Buffer<float>(cli,tmp);
#endif
            //mesh.hasNormals=false;
            mesh.hasNormals=true;
            dout<<"mesh vbo = "<<mesh.vbo<<endl;
            dout<<"normal vbo = "<<mesh.normalbo<<endl;
            glFinish();
        }
        mesh.vboSize=total*3;
        if(total!=0)
        {
            iarg=0;
#if 1
            cl_triangles.acquire();
            cl_normals.acquire();
#endif
            //dout<<"Num_args = "<<k_traverse[levels-1].kernel.getInfo<CL_KERNEL_NUM_ARGS>()<<endl;
            for(int j = 0; j<levels; j++)
            {
                k_traverse[levels-1].setArg(iarg++,cl_histopyramid[j]);
            }
            k_traverse[levels-1].setArg(iarg++,cl_triangles.getDevicePtr());
            k_traverse[levels-1].setArg(iarg++,cl_normals.getDevicePtr());
            k_traverse[levels-1].setArg(iarg++,res);
            k_traverse[levels-1].setArg(iarg++,slices);
            k_traverse[levels-1].setArg(iarg++,isolevel);
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

#if 0
            vector<float> norms=cl_normals.copyToHost(total*3);
            float avg[3]={0.0f,0.0f,0.0f};
            for(unsigned int j=0;j<total; j++)
            {
                if(norms[(j*3)]>0.0f && norms[(j*3)]<1.0f)
                {
                    dout<<"Found one"<<endl;
                    avg[0]+=norms[(j*3)];
                    avg[1]+=norms[(j*3)+1];
                    avg[2]+=norms[(j*3)+2];
                }
            }
            avg[0]/=total;
            avg[1]/=total;
            avg[2]/=total;
            dout<<"The average normal is ("<<avg[0]<<","<<avg[1]<<","<<avg[2]<<")"<<endl;
#endif
#if 1
            cl_triangles.release();
            cl_normals.release();
#endif
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

