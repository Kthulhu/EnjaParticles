#ifndef TEST_APPLICATION_H
#define TEST_APPLICATION_H
#include <map>
#include <string>
#include <iostream>

#include "../rtpslib/system/System.h"
#include "../rtpslib/system/ParticleShape.h"
#include "../rtpslib/render/ParticleEffect.h"
#include <assimp/assimp.h>
#include <assimp/aiPostProcess.h>
#include <assimp/aiScene.h>

#define aisgl_min(x,y) (x<y?x:y)
#define aisgl_max(x,y) (y>x?y:x)
namespace rtps
{
    struct Mesh
    {
        aiMatrix4x4 modelMat;
        GLuint vbo;
        GLuint colbo;
        GLuint ibo;
        unsigned int vboSize;
        unsigned int iboSize;
    };
   class TestApplication
    {
        public:
            TestApplication(std::istream& is);
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
            void readParamFile(std::istream& is);
            GLuint getWindowHeight() const;
            GLuint getWindowWidth() const;
            void setWindowHeight(GLuint windowHeight);
            void setWindowWidth(GLuint windowWidth);
            void loadScene(std::string& filename);
            //FIXME: These following methods are used for assimp to import modules. They should
            //probably be better. Currently they are almost verbatim from a loading example.
            void display();
            void build_shapes (const struct aiScene *sc, const struct aiNode* nd, struct aiMatrix4x4 parentTransform=aiMatrix4x4(1.0f,0.0f,0.0f,0.0f,
                                                                                                                            0.0f,1.0f,0.0f,0.0f,
                                                                                                                            0.0f,0.0f,1.0f,0.0f,
                                                                                                                            0.0f,0.0f,0.0f,1.0f) );
            void recursive_render (const struct aiScene *sc, const struct aiNode* nd);
            void apply_material(const struct aiMaterial *mtl);
            void set_float4(float f[4], float a, float b, float c, float d);
            void color4_to_float4(const struct aiColor4D *c, float f[4]);
            void get_bounding_box (struct aiVector3D* min, struct aiVector3D* max);
            void get_bounding_box_for_node (const struct aiNode* nd,
                    struct aiVector3D* min,
                    struct aiVector3D* max,
                    struct aiMatrix4x4* trafo
                );

        private:
            GLuint windowWidth,windowHeight;
            std::map<std::string,System*> systems;
            std::map<std::string,ParticleEffect*> effects;
            std::map<std::string,ParticleShape*> pShapes;
            std::map<std::string,Mesh*> meshs;
            ShaderLibrary* lib;
            std::string renderType;
            float3 rotation; //may want to consider quaternions for this at some point.
            float3 translation;
            int2 mousePos;
            int mouseButtons;
            // the global Assimp scene object
            const struct aiScene* scene;
            GLuint scene_list;
            struct aiVector3D scene_min, scene_max, scene_center;


            float4 gridMax,gridMin;
            float sizeScale;
            float mass;
            CL* cli;
            bool paused,renderVelocity;
    };
};
#endif
