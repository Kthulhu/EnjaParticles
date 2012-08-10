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
            TestApplication(std::string path,GLuint w, GLuint h);
            ~TestApplication();
            virtual void KeyboardCallback(unsigned char key, int x, int y);
            virtual void RenderCallback();
            virtual void DestroyCallback();
            virtual void MouseCallback(int button, int state, int x, int y);
            virtual void MouseMotionCallback(int x, int y);
            virtual void ResizeWindowCallback(int w, int h);
            virtual void TimerCallback(int ms);
            virtual void ResetSimulations();
            virtual void drawString(const char *str, int x, int y, float color[4], void *font);
            virtual void initGL();
            virtual void readParamFile(std::istream& is, std::string path);
            virtual void setWindowHeight(GLuint windowHeight);
            virtual void setWindowWidth(GLuint windowWidth);
            virtual void loadScene(const std::string& filename);
            virtual void loadMeshScene(const std::string& filename);
            virtual void addRigidBody(const std::string& system, const std::string& mesh, float4 pos, float4 vel, float mass);
	    virtual void renderSkyBox();
	    virtual void initParams(std::istream& is);
	    virtual void initScenes();
	    virtual void cameraChanged();
	    ParticleShape* createParticleShape(const std::string& system, Mesh* mesh, float scale =1.0f);
            //FIXME: These following methods are used for assimp to import modules. They should
            //probably be better. Currently they are almost verbatim from a loading example.
            virtual void display(bool blend);
	    virtual void createSceneTextures();

            int writeMovieFrame(const char* filename, const char* dir);
	GLuint width(){return windowWidth;}
	GLuint height(){return windowHeight;}

        protected:
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
        GLuint sceneFBO;
        GLuint sceneTex[4]; ///store scene color buffer and scene depth buffer need quad buffer for stereo
	bool stereoscopic;
  
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
	std::string currentMesh;
    };
};
#endif
