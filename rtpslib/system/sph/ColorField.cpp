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


#include <SPH.h>

#include "../../render/util/stb_image_write.h"

namespace rtps
{

   /* ColorField::ColorField(const ColorField& cf)
    {
        cli = cf.cli;
        timer = cf.timer;
        cl_colField=cf.cl_colField;
        res = cf.res;
        texRes2D=cf.texRes2D;
        slices=cf.slices;
    }*/
    //----------------------------------------------------------------------
    ColorField::ColorField(std::string path, CL* cli_, EB::Timer* timer_,unsigned int res)
    {
        cli = cli_;
        timer = timer_;
        if(res<2)
            this->res=2;
        else
            this->res=res;

        printf("load colorfield\n");

        try
        {
            path = path + "/colorfield.cl";
            k_colorfield = Kernel(cli, path, "colorfield_update");
        }
        catch (cl::Error er)
        {
            printf("ERROR(ColorField): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
        }

        initializeData();

    }
    //----------------------------------------------------------------------

    void ColorField::initializeData()
    {
        slices = 1u<<static_cast<unsigned int>(ceil(log(ceil(sqrt((float)res))/log(2.0f))));
        texRes2D = res*slices;
        float* zeroImg = new float[texRes2D*texRes2D];
        memset(zeroImg,0,texRes2D*texRes2D*sizeof(float));
        //dout<<"-----------------texRes2D "<<texRes2D<<endl;
        cl_colField=cl::Image2D(cli->context,CL_MEM_READ_WRITE,cl::ImageFormat(CL_R, CL_FLOAT),texRes2D,texRes2D,0,zeroImg);
        cli->queue.finish();
        delete[] zeroImg;
    }
    cl::Image2D ColorField::execute(Buffer<float4>& pos_s,
                    Buffer<float>& dens_s,
                    int res,
                    Buffer<unsigned int>& ci_start,
                    Buffer<unsigned int>& ci_end,
                    //params
                    Buffer<SPHParams>& sphp,
                    Buffer<GridParams>& gp,
                    //debug params
                    Buffer<float4>& clf_debug,
                    Buffer<int4>& cli_debug)
    {
        if(this->res!=res)
        {
            if(res<2)
                this->res=2;
            else
                this->res=res;
            initializeData();
        }
        int iarg = 0;

        //dout<<"res = "<<res<<endl;
        k_colorfield.setArg(iarg++, res);
        k_colorfield.setArg(iarg++, slices);
        k_colorfield.setArg(iarg++, pos_s.getDevicePtr());
        k_colorfield.setArg(iarg++, dens_s.getDevicePtr());
        k_colorfield.setArg(iarg++, cl_colField);
        k_colorfield.setArg(iarg++, ci_start.getDevicePtr());
        k_colorfield.setArg(iarg++, ci_end.getDevicePtr());
        k_colorfield.setArg(iarg++, gp.getDevicePtr());
        k_colorfield.setArg(iarg++, sphp.getDevicePtr());

        // ONLY IF DEBUGGING
        k_colorfield.setArg(iarg++, clf_debug.getDevicePtr());
        k_colorfield.setArg(iarg++, cli_debug.getDevicePtr());

        int local = 64;
        try
        {
            float gputime = k_colorfield.execute(cl::NDRange(res,res,res));
            if(gputime > 0)
                timer->set(gputime);

        }

        catch (cl::Error er)
        {
            printf("ERROR(force ): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
        }


#if 0 //printouts
        //DEBUGING

        int num =res*res*res;
        if(num > 0)// && choice == 0)
        {
            printf("============================================\n");
            printf("***** PRINT colorfield diagnostics ******\n");
            printf("num %d\n", num);

            std::vector<int4> cli(num);
            std::vector<float4> clf(num);

            cli_debug.copyToHost(cli);
            clf_debug.copyToHost(clf);

            std::vector<float4> poss(num);
            std::vector<float4> dens(num);
            float t3=0.0f;
            for(int j = 0;j<num;j++)
            {
                t3+=clf[j].x;
            }
            dout<<"Total active points = "<<t3<<endl;


            for (int i=0; i < num; i++)
            {
                if(clf[i].x)
                {
                    //printf("clf_debug: %f, %f, %f, %f\n", clf[i].x, clf[i].y, clf[i].z, clf[i].w);
                }
            }
            dout<<"texRes2D = "<<texRes2D<<endl;
        }
        //DEBUGGING!!
        try
        {
            float* tmpImg = new float[texRes2D*texRes2D*4];
            memset(tmpImg,0,texRes2D*texRes2D*4*sizeof(float));
            cl::size_t<3> origin;
            origin[0]=0;origin[1]=0;origin[2]=0;
            cl::size_t<3> tmpregion;
            tmpregion[0]=texRes2D;
            tmpregion[1]=texRes2D;
            tmpregion[2]=1;
            float t2=0.0f;
            for(int j = 0;j<texRes2D*texRes2D*4;j+=4)
            {
     //           if(tmpImg[j]<0.0f || tmpImg[j]>5.0f)
     //               dout<<"WTF!!"<<endl;
     //           t2+=tmpImg[j];
                dout<<tmpImg[j]<<","<<tmpImg[j+1]<<","<<tmpImg[j+2]<<","<<tmpImg[j+3]<<endl;
            }
            dout<<"Total active points = "<<t2<<endl;

            cli->queue.enqueueReadImage(cl_colField, CL_TRUE, origin, tmpregion, 0, 0, tmpImg);
            dout<<"Here"<<endl;
            cli->queue.finish();
            float t=0.0f;
            unsigned char* image = new unsigned char[texRes2D*texRes2D*4];
            for(int j = 0;j<texRes2D*texRes2D*4;j+=1)
            {
                //if(tmpImg[j]<0.0f || tmpImg[j]>5.0f)
                //    dout<<"WTF!!"<<endl;
                if(j%4==0)
                    t+=tmpImg[j];
                image[j]=(unsigned char)tmpImg[j];
            }
            delete[] tmpImg;
            if (!stbi_write_png("/home/andrew/Desktop/colorfield.png",texRes2D,texRes2D,4,(void*)image,0))
            {
                cout<<"failed to write image "<<endl;
            }
            delete[]image;

            dout<<"Total active points = "<<t<<endl;
        }
        catch (cl::Error er)
        {
            printf("ERROR(marchingcubes ): %s(%s)\n", er.what(), CL::oclErrorString(er.err()));
        }

#endif
        return cl_colField;
    }


}

