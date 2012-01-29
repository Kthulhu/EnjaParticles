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
#include "TestApplication.h"

using namespace rtps;
using namespace std;
TestApplication* app=NULL;
int glutWindowHandle = 0;


void appKeyboard(unsigned char key, int x, int y)
{
    app->KeyboardCallback(key,x,y);
}
void appRender()
{
    app->RenderCallback();
}
void appDestroy()
{
    delete app;
}
void appMouse(int button, int state, int x, int y)
{
    app->MouseCallback(button,state,x,y);
}
void appMotion(int x, int y)
{
    app->MouseMotionCallback(x,y);
}
void resizeWindow(int w, int h)
{
    app->ResizeWindowCallback(w,h);
}
void timerCB(int ms)
{
    glutTimerFunc(ms, timerCB, ms);
    app->TimerCallback(ms);
}
//----------------------------------------------------------------------
int main(int argc, char** argv)
{
    string paramFile="test.xml";
    //initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_ALPHA //| GLUT_ALPHA| GLUT_INDEX
		//|GLUT_STEREO //if you want stereo you must uncomment this.
		);
    if(argc>1)
        paramFile = argv[1];
    dout<<paramFile<<endl;
        
    ifstream is(paramFile.c_str(),ifstream::in);
    app=new TestApplication(is);
    glutInitWindowSize(app->getWindowWidth(), app->getWindowHeight());
    glutInitWindowPosition (glutGet(GLUT_SCREEN_WIDTH)/2 - app->getWindowWidth()/2, 
                            glutGet(GLUT_SCREEN_HEIGHT)/2 - app->getWindowHeight()/2);


    std::stringstream ss;
    ss << "Real-Time Particle System " << std::ends;
    glutWindowHandle = glutCreateWindow(ss.str().c_str());

    glutDisplayFunc(appRender); //main rendering function
    glutTimerFunc(30, timerCB, 30); //determin a minimum time between frames
    glutKeyboardFunc(appKeyboard);
    glutMouseFunc(appMouse);
    glutMotionFunc(appMotion);
    glutReshapeFunc(resizeWindow);

    glutMainLoop();
    return 0;
}



