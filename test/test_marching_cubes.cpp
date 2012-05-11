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

#include <fstream>
#include <string>
#include <GL/glew.h>
#if defined __APPLE__ || defined(MACOSX)
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
//OpenCL stuff
#endif

//#include "BunnyMesh.h"
#include "../rtpslib/RTPS.h"
#include "../rtpslib/system/common/MarchingCubes.h"
#include "../rtpslib/timer_eb.h"
#include "../rtpslib/util.h"
using namespace rtps;
using namespace std;
int glutWindowHandle = 0;
MarchingCubes* mc = NULL;
CL* cli = NULL;
EB::Timer* eb = NULL;
unsigned long res = 64;
string filepath;

void cleanUp()
{
    delete cli;
    delete mc;
    delete eb;
}
void appKeyboard(unsigned char key, int x, int y)
{
    if(key =='q')
    {
        exit(0);
    }
    if(key == '+')
    {
        res*=2;
        delete mc;
        cout<< "creating new marching cubes instance with res = "<<res<<endl;
        mc = new MarchingCubes(filepath+"/cl_common",cli,eb,res);
        cout<< "finished creating new marching cubes instance with res = "<<res<<endl;
    }
    if(key == '-')
    {
        res/=2;
        delete mc;
        cout<< "creating new marching cubes instance with res = "<<res<<endl;
        mc = new MarchingCubes(filepath+"/cl_common",cli,eb,res);
        cout<< "finished creating new marching cubes instance with res = "<<res<<endl;
    }
}
void appRender()
{
}
void appMouse(int button, int state, int x, int y)
{
}
void appMotion(int x, int y)
{
}
void resizeWindow(int w, int h)
{
}
void timerCB(int ms)
{
    glutTimerFunc(ms, timerCB, ms);
    glutPostRedisplay();
}
//----------------------------------------------------------------------
int main(int argc, char** argv)
{
    //initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_ALPHA //| GLUT_ALPHA| GLUT_INDEX
            | GLUT_MULTISAMPLE
		//|GLUT_STEREO //if you want stereo you must uncomment this.
		);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition (glutGet(GLUT_SCREEN_WIDTH)/2 - 800/2,
                            glutGet(GLUT_SCREEN_HEIGHT)/2 - 600/2);
    std::stringstream ss;
    ss << "Real-Time Particle System " << std::ends;
    glutWindowHandle = glutCreateWindow(ss.str().c_str());

    glutDisplayFunc(appRender); //main rendering function
    glutTimerFunc(30, timerCB, 30); //determin a minimum time between frames
    glutKeyboardFunc(appKeyboard);
    glutMouseFunc(appMouse);
    glutMotionFunc(appMotion);
    glutReshapeFunc(resizeWindow);
	filepath=argv[0];
	unsigned int pos=0;
#ifdef WIN32
	pos=filepath.rfind("\\");
#else
	pos=filepath.rfind("/");
#endif
	filepath=filepath.substr(0,pos+1);
	filepath=filepath.substr(0,filepath.size()-1);

    cli = new CL();
    eb=new EB::Timer();
    mc = new MarchingCubes(filepath+"/cl_common",cli,eb,res);
    atexit(cleanUp);
    glutMainLoop();
    return 0;
}



