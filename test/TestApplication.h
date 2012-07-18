#ifndef TEST_APPLICATION_H
#define TEST_APPLICATION_H
#include <map>
#include <string>
#include <iostream>

#include "../rtpslib/RTPS.h"
#include "../rtpslib/render/MeshEffect.h"
#include "../rtpslib/system/System.h"
#include "../rtpslib/system/ParticleShape.h"
#include "../rtpslib/render/ParticleEffect.h"

namespace rtps
{

	//class System;
	class AIWrapper;
	//class MeshEffect;
	//class ParticleEffect;
	//class ShaderLibrary;
	//class CL;
    class TestApplication
    {


        public:
            TestApplication(std::istream& is, std::string path,GLuint w, GLuint h);
            ~TestApplication();
            void KeyboardCallback(unsigned char key, int x, int y);
            void RenderCallback();
            void DestroyCallback();
            void MouseCallback(int button, int state, int x, int y);
            void MouseMotionCallback(int x, int y);
            void ResizeWindowCallback(int w, int h);
            void TimerCallback(int ms);
            void ResetSimulations();
            void drawString(const char *str, int x, int y, float color[4], void *font);
            void initGL();
            void readParamFile(std::istream& is, std::string path);
            void setWindowHeight(GLuint windowHeight);
            void setWindowWidth(GLuint windowWidth);
            void loadScene(const std::string& filename);
            void loadMeshScene(const std::string& filename);
	    void renderSkyBox();
	    void cameraChanged();
	    ParticleShape* createParticleShape(const std::string& system, Mesh* mesh);
            //FIXME: These following methods are used for assimp to import modules. They should
            //probably be better. Currently they are almost verbatim from a loading example.
            void display(bool blend);

            int writeMovieFrame(const char* filename, const char* dir);
	GLuint width(){return windowWidth;}
	GLuint height(){return windowHeight;}

        private:
            GLuint windowWidth,windowHeight, environTex;

        std::map<std::string,System*> systems;
        std::map<std::string,std::string> systemRenderType;
        std::map<std::string,ParticleEffect*> effects;
        MeshEffect* meshRenderer;
        std::map<std::string,ParticleShape*> pShapes;
        std::map<std::string,Mesh* > meshes;
        std::map<std::string,Mesh* > dynamicMeshes;
        ShaderLibrary* lib;

        Camera* view;
        Light* light;
        int2 mousePos;
        int mouseButtons;

        AIWrapper* scene;
        AIWrapper* dynamicMeshScene;
        //GLuint scene_list;
        GLuint skyboxVBO,skyboxTexVBO;

        int frameCounter;
        bool renderMovie;

        float4 gridMax,gridMin;
        float sizeScale;
        float mass;
        CL* cli;
        bool paused,renderVelocity;
        std::string binaryPath;
    };
};
#endif
