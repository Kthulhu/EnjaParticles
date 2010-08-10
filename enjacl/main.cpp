//
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>


//#include <utils.h>

//#include <string.h>
//#include <string>
#include <sstream>
#include <iomanip>

#include <GL/glew.h>
#if defined __APPLE__ || defined(MACOSX)
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
    //OpenCL stuff
#endif

#include "enja.h"
#include "timege.h"

int window_width = 800;
int window_height = 600;
int glutWindowHandle = 0;
float translate_z = -4.f;

// mouse controls
int mouse_old_x, mouse_old_y;
int mouse_buttons = 0;
float rotate_x = 0.0, rotate_y = 0.0;
std::vector<Triangle> triangles;
std::vector<Box> boxes;

// offsets into the triangle list. tri_offsets[i] corresponds to the 
// triangle list for box[i]. Number of triangles for triangles[i] is
//    tri_offsets[i+1]-tri_offsets[i]
// Add one more offset so that the number of triangles in 
//   boxes[boxes.size()-1] is tri_offsets[boxes.size()]-tri_offsets[boxes.size()-1]
std::vector<int> tri_offsets;


void init_gl();

void appKeyboard(unsigned char key, int x, int y);
void appRender();
void appDestroy();

void appMouse(int button, int state, int x, int y);
void appMotion(int x, int y);

void timerCB(int ms);

void drawString(const char *str, int x, int y, float color[4], void *font);
void showFPS(float fps, std::string *report);
void *font = GLUT_BITMAP_8_BY_13;

EnjaParticles* enjas;
#define NUM_PARTICLES 1024*16


GLuint v_vbo; //vbo id
GLuint c_vbo; //vbo id

//timers
GE::Time *ts[3];

//================
#include "materials_lights.h"

//----------------------------------------------------------------------
float rand_float(float mn, float mx)
{
	float r = random() / (float) RAND_MAX;
	return mn + (mx-mn)*r;
}
//----------------------------------------------------------------------
void make_cube(Vec4 cen, float half_edge)
{
// Written by G. Erlebacher Aug. 5, 2010
/*

        7-----------6 
       /           /|
      /           / |           Z
     4-----------5  |           |
	 |           |  2           |  Y
	 |           | /            | /
	 |           |/             |/
     0-----------1              x------- X
	              
*/
	//printf("inside make_cube\n");
	// vertices
	std::vector<Vec4> v;
	float h = half_edge;
	Vec4 vv;

	vv.set(cen.x-h, cen.y-h, cen.z-h);
	v.push_back(vv);
	vv.set(cen.x+h, cen.y-h, cen.z-h);
	v.push_back(vv);
	vv.set(cen.x+h, cen.y+h, cen.z-h);
	v.push_back(vv);
	vv.set(cen.x-h, cen.y+h, cen.z-h);
	v.push_back(vv);
	vv.set(cen.x-h, cen.y-h, cen.z+h);
	v.push_back(vv);
	vv.set(cen.x+h, cen.y-h, cen.z+h);
	v.push_back(vv);
	vv.set(cen.x+h, cen.y+h, cen.z+h);
	v.push_back(vv);
	vv.set(cen.x-h, cen.y+h, cen.z+h);
	v.push_back(vv);

	// Boxes
	Box box;
	box.xmin = cen.x-h;
	box.xmax = cen.x+h;
	box.ymin = cen.y-h;
	box.ymax = cen.y+h;
	box.zmin = cen.z-h;
	box.zmax = cen.z+h;
	boxes.push_back(box);

	// Triangles
	Triangle tri;

	tri.verts[0] = v[2];
	tri.verts[1] = v[1];
	tri.verts[2] = v[0];
	tri.normal.set(0.,0.,-1.,0.);
	triangles.push_back(tri);

	tri.verts[0] = v[3];
	tri.verts[1] = v[2];
	tri.verts[2] = v[0];
	tri.normal.set(0.,0.,-1.,0.);
	triangles.push_back(tri);
	//printf("triangles: size: %d\n", triangles.size());

	tri.verts[0] = v[4];
	tri.verts[1] = v[5];
	tri.verts[2] = v[6];
	tri.normal.set(0.,0.,+1.,0.);
	triangles.push_back(tri);

	tri.verts[0] = v[4];
	tri.verts[1] = v[6];
	tri.verts[2] = v[7];
	tri.normal.set(0.,0.,+1.,0.);
	triangles.push_back(tri);

	//---
	tri.verts[0] = v[0];
	tri.verts[1] = v[1];
	tri.verts[2] = v[5];
	tri.normal.set(0.,-1.,0.,0.);
	triangles.push_back(tri);

	tri.verts[0] = v[0];
	tri.verts[1] = v[5];
	tri.verts[2] = v[4];
	tri.normal.set(0.,-1.,0.,0.);
	triangles.push_back(tri);

	tri.verts[0] = v[7];
	tri.verts[1] = v[6];
	tri.verts[2] = v[2];
	tri.normal.set(0.,+1.,0.,0.);
	triangles.push_back(tri);

	tri.verts[0] = v[7];
	tri.verts[1] = v[2];
	tri.verts[2] = v[3];
	tri.normal.set(0.,+1.,0.,0.);
	triangles.push_back(tri);

	//----
	tri.verts[0] = v[1];
	tri.verts[1] = v[2];
	tri.verts[2] = v[6];
	tri.normal.set(+1.,0.,0.,0.);
	triangles.push_back(tri);

	tri.verts[0] = v[1];
	tri.verts[1] = v[6];
	tri.verts[2] = v[5];
	tri.normal.set(+1.,0.,0.,0.);
	triangles.push_back(tri);

	tri.verts[0] = v[0];
	tri.verts[1] = v[4];
	tri.verts[2] = v[7];
	tri.normal.set(-1.,0.,0.,0.);
	triangles.push_back(tri);

	tri.verts[0] = v[0];
	tri.verts[1] = v[7];
	tri.verts[2] = v[3];
	tri.normal.set(-1.,0.,0.,0.);
	triangles.push_back(tri);
}
//----------------------------------------------------------------------

//load texture from bmp file
/*
//image handling constructor being tested here
EnjaParticles::EnjaParticles(int s, const char* img_filename)
{
    printf("in tha constructor!\n");
    system = s;
    particle_radius = 5.0f;

    //load the image with OpenCV
    Mat img = imread(img_filename, 1);
    //convert from BGR to RGB colors
    cvtColor(img, img, CV_BGR2RGB);
    //this is ugly but it makes an iterator over our 
    MatIterator_<Vec<uchar, 3> > it = img.begin<Vec<uchar,3> >(), it_end = img.end<Vec<uchar,3> >();
    int w = img.size().width;
    int h = img.size().height;
    int n = w * h;

    AVec4 g(n);
    AVec4 v(n);
    AVec4 c(n);
    int i = 0;
    float f = 0;
    //for(; itr != itr_end; ++itr, ++itg, ++itb)
    for(; it != it_end; ++it)
    {
        //printf("i: %d\n");
        //printf("color %d %d %d\n", it[0][0], it[0][1], it[0][2]);
        c[i].x = it[0][0]/255.0f;
        c[i].y = it[0][1]/255.0f;
        c[i].z = it[0][2]/255.0f;
        c[i].w = 1.0f;
        //printf("color %g %g %g\n", c[i].x, c[i].y, c[i].z);
        
        f = (float)i;
        g[i].x = (i%w) * (1.0f/w) - .5f;
        g[i].y = (i/w) * (1.0f/h) - .5f;
        g[i].z = 0.0f;
        g[i].w = 1.0f;
        //printf("pos %g %g %g\n", g[i].x, g[i].y, g[i].z);
        
        v[i].x = 0.0;// + .5*cos(2.*M_PI*(f/n));  
        v[i].y = 0.0;// + .5*sin(2.*M_PI*(f/n));
        v[i].z = 0.f;
        v[i].w = 0.f;

        i++;
    }
    printf("i: %d, n: %d\n", i, n);
    //printf("%g %g %g\n", img(0,0)[0], img(0,0)[1], img(0,0)[2]);

    srand48(time(NULL));
    printf("about to init\n");
    init(g, v, c, n);


}
*/
//should switch to blender's library, or just pass in the texture from blender
#include "highgui.h"
#include "cv.h"
using namespace cv;
int main(int argc, char** argv)
{

    //initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(window_width, window_height);
    glutInitWindowPosition (glutGet(GLUT_SCREEN_WIDTH)/2 - window_width/2, 
                            glutGet(GLUT_SCREEN_HEIGHT)/2 - window_height/2);

    
    std::stringstream ss;
    ss << "EnjaParticles: " << NUM_PARTICLES << std::ends;
    glutWindowHandle = glutCreateWindow(ss.str().c_str());

    glutDisplayFunc(appRender); //main rendering function
    glutTimerFunc(30, timerCB, 30); //determin a minimum time between frames
    glutKeyboardFunc(appKeyboard);
    glutMouseFunc(appMouse);
    glutMotionFunc(appMotion);

	define_lights_and_materials();

    // initialize necessary OpenGL extensions
    glewInit();
    GLboolean bGLEW = glewIsSupported("GL_VERSION_2_0 GL_ARB_pixel_buffer_object"); 
    printf("GLEW supported?: %d\n", bGLEW);

    //initialize the OpenGL scene for rendering
    init_gl();

    printf("before we call enjas functions\n");

    //parameters: system and number of particles
    //system = 0: lorenz
    //system = 1 gravity
    //system = 2 vfield

    
    //default constructor
    enjas = new EnjaParticles(EnjaParticles::GRAVITY, NUM_PARTICLES);
    enjas->particle_radius = 10.0f;
    
    enjas->use_glsl();
    
    //load the image with OpenCV
    std::string path(CL_SOURCE_DIR);
    path += "/tex/particle.jpg";
    Mat img = imread(path, 1);
    //Mat img = imread("tex/enjalot.jpg", 1);
    //convert from BGR to RGB colors
    //cvtColor(img, img, CV_BGR2RGB);
    //this is ugly but it makes an iterator over our image data
    //MatIterator_<Vec<uchar, 3> > it = img.begin<Vec<uchar,3> >(), it_end = img.end<Vec<uchar,3> >();
    MatIterator_<Vec<uchar, 3> > it = img.begin<Vec<uchar,3> >(), it_end = img.end<Vec<uchar,3> >();
    int w = img.size().width;
    int h = img.size().height;
    int n = w * h;
    std::vector<unsigned char> tex;//there are n bgr values 

    printf("read image data %d \n", n);
    for(; it != it_end; ++it)
    {
   //     printf("asdf: %d\n", it[0][0]);
        tex.push_back(it[0][0]);
        tex.push_back(it[0][1]);
        tex.push_back(it[0][2]);
    }
    //unsigned char* asdf = &tex[0];
    //for(int i = 0; i < 3*n; i+=3)
    //{
    //    printf("b: %d", asdf[i]);
    //}
        
    enjas->loadTexture( tex, w, h );

    enjas->updates = 1;
    enjas->dt = .001;
    enjas->collision = true;


// make cubes, formed from triangles

	int nb_cubes = 100;
	Vec4 cen;

	tri_offsets.push_back(0);

	for (int i=0; i < nb_cubes; i++) {
		float rx = rand_float(-1.5,1.5);
		float ry = rand_float(-1.5,1.5);
		float rz = rand_float(-5.,0.);
		cen.set(rx,ry,rz,1.);
		make_cube(cen, 0.1);
		tri_offsets.push_back(triangles.size());
	}

	// once all triangles are created: 
	tri_offsets.push_back(triangles.size());

    //enjas->transform[0] = Vec4(1,0,0,0);
    //enjas->loadTriangles(triangles);
    enjas->loadBoxes(boxes, triangles, tri_offsets);
    
   
    //Test making a system from vertices and normals;
    /*
    Vec4 g[4];
    Vec4 v[4];
    g[0] = Vec4(0.0f, -1.0f, 0.0f, 1.0f);
    g[1] = Vec4(0.0f, 1.0f, 0.0f, 1.0f);
    g[2] = Vec4(1.0f, 0.0f, 0.0f, 1.0f);
    g[3] = Vec4(-1.0f, 0.0f, 0.0f, 1.0f);

    v[0] = Vec4(0.0f, 0.0f, 1.0f, 0.0f);
    v[1] = Vec4(0.0f, 0.0f, 1.0f, 0.0f);
    v[2] = Vec4(0.0f, 0.0f, 1.0f, 0.0f);
    v[3] = Vec4(0.0f, 0.0f, 1.0f, 0.0f);

    enjas = new EnjaParticles(1, g, v, 4, NUM_PARTICLES);
    */


    glutMainLoop();
    
    printf("doesn't happen does it\n");
    appDestroy();
    return 0;
}



void init_gl()
{
    // default initialization
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glDisable(GL_DEPTH_TEST);

    // viewport
    glViewport(0, 0, window_width, window_height);

    // projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //gluPerspective(60.0, (GLfloat)window_width / (GLfloat) window_height, 0.1, 100.0);
    gluPerspective(90.0, (GLfloat)window_width / (GLfloat) window_height, 0.1, 10000.0); //for lorentz

    // set view matrix
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, translate_z);
    //glRotatef(-90, 1.0, 0.0, 0.0);

    return;

}

void appKeyboard(unsigned char key, int x, int y)
{
    switch(key) 
    {
        case '\033': // escape quits
        case '\015': // Enter quits    
        case 'Q':    // Q quits
        case 'q':    // q (or escape) quits
            // Cleanup up and quit
            appDestroy();
            break;
    }
}

void appRender()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
/*
    plane[0] = (float4)(-2,-2,-1,0);
    plane[1] = (float4)(-2,2,-1,0);
    plane[2] = (float4)(2,2,-1,0);
    plane[3] = (float4)(2,-2,-1,0);

    Vec4 plane[4];
    plane[0] = Vec4(-5,-5,-1,0);
    plane[1] = Vec4(-5,5,-1,0);
    plane[2] = Vec4(10,2,-3,0);
    plane[3] = Vec4(5,-5,-1,0);
*/

	glEnable(GL_DEPTH_TEST);


    glBegin(GL_TRIANGLES);
    glColor3f(0,1,0);
	for (int i=0; i < triangles.size(); i++) {
	//for (int i=0; i < 20; i++) {
		Triangle& tria = triangles[i];
		glNormal3fv(&tria.normal.x);
    	glVertex3f(tria.verts[0].x, tria.verts[0].y, tria.verts[0].z);
    	glVertex3f(tria.verts[1].x, tria.verts[1].y, tria.verts[1].z);
    	glVertex3f(tria.verts[2].x, tria.verts[2].y, tria.verts[2].z);
	}
    glEnd();

    glColor3f(0,0,0);

 
    enjas->render();

    showFPS(enjas->getFPS(), enjas->getReport());
    glutSwapBuffers();
    //if we want to render as fast as possible we do this
    //glutPostRedisplay();

	glDisable(GL_DEPTH_TEST);
}

void appDestroy()
{

    delete enjas;
    if(glutWindowHandle)glutDestroyWindow(glutWindowHandle);
    printf("about to exit!\n");

    exit(0);
}

void timerCB(int ms)
{
    glutTimerFunc(ms, timerCB, ms);
    glutPostRedisplay();
}


void appMouse(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN) {
        mouse_buttons |= 1<<button;
    } else if (state == GLUT_UP) {
        mouse_buttons = 0;
    }

    mouse_old_x = x;
    mouse_old_y = y;

    glutPostRedisplay();
}

void appMotion(int x, int y)
{
    float dx, dy;
    dx = x - mouse_old_x;
    dy = y - mouse_old_y;

    if (mouse_buttons & 1) {
        rotate_x += dy * 0.2;
        rotate_y += dx * 0.2;
    } else if (mouse_buttons & 4) {
        translate_z += dy * 0.1;
    }

    mouse_old_x = x;
    mouse_old_y = y;

    // set view matrix
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, translate_z);
    glRotatef(rotate_x, 1.0, 0.0, 0.0);
    glRotatef(rotate_y, 0.0, 1.0, 0.0);
    glutPostRedisplay();
}


///////////////////////////////////////////////////////////////////////////////
// write 2d text using GLUT
// The projection matrix must be set to orthogonal before call this function.
///////////////////////////////////////////////////////////////////////////////
void drawString(const char *str, int x, int y, float color[4], void *font)
{
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
    glDisable(GL_LIGHTING);     // need to disable lighting for proper text color

    glColor4fv(color);          // set text color
    glRasterPos2i(x, y);        // place text position

    // loop all characters in the string
    while(*str)
    {
        glutBitmapCharacter(font, *str);
        ++str;
    }

    glEnable(GL_LIGHTING);
    glPopAttrib();
}

///////////////////////////////////////////////////////////////////////////////
// display frame rates
///////////////////////////////////////////////////////////////////////////////
void showFPS(float fps, std::string* report)
{
    static std::stringstream ss;

    // backup current model-view matrix
    glPushMatrix();                     // save current modelview matrix
    glLoadIdentity();                   // reset modelview matrix

    // set to 2D orthogonal projection
    glMatrixMode(GL_PROJECTION);        // switch to projection matrix
    glPushMatrix();                     // save current projection matrix
    glLoadIdentity();                   // reset projection matrix
    gluOrtho2D(0, 400, 0, 300);         // set to orthogonal projection

    float color[4] = {1, 1, 0, 1};

    // update fps every second
    ss.str("");
    ss << std::fixed << std::setprecision(1);
    ss << fps << " FPS" << std::ends; // update fps string
    ss << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);
    drawString(ss.str().c_str(), 15, 286, color, font);
    drawString(report[0].c_str(), 15, 273, color, font);
    drawString(report[1].c_str(), 15, 260, color, font);

    // restore projection matrix
    glPopMatrix();                      // restore to previous projection matrix

    // restore modelview matrix
    glMatrixMode(GL_MODELVIEW);         // switch to modelview matrix
    glPopMatrix();                      // restore to previous modelview matrix
}
//----------------------------------------------------------------------
