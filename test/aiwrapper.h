
#ifndef AIWRAPPER_H
#define AIWRAPPER_H

#include <assimp/assimp.h>
#include <assimp/aiPostProcess.h>
#include <assimp/aiScene.h>
#include <QString>
#include <map>
#include <string>
#include <iostream>

//Mesh is defined in mesh effect
#include "../rtpslib/render/MeshEffect.h"
#define aisgl_min(x,y) (x<y?x:y)
#define aisgl_max(x,y) (y>x?y:x)

namespace rtps
{
class AIWrapper
{
public:
    const struct aiScene* getScene();
    void loadMeshes (std::map<QString,Mesh*>& meshes, const struct aiNode* nd, struct aiMatrix4x4 parentTransform=aiMatrix4x4(1.0f,0.0f,0.0f,0.0f,
                                                                                                                    0.0f,1.0f,0.0f,0.0f,
                                                                                                                    0.0f,0.0f,1.0f,0.0f,
                                                                                                                    0.0f,0.0f,0.0f,1.0f) );
    void recursive_render (const struct aiNode* nd);
    void apply_material(const struct aiMaterial *mtl, Mesh* mesh);
    void set_float4(float f[4], float a, float b, float c, float d);
    void color4_to_float4(const struct aiColor4D *c, float f[4]);
    void get_bounding_box (struct aiVector3D* min, struct aiVector3D* max);
    void get_bounding_box_for_node (const struct aiNode* nd,
            struct aiVector3D* min,
            struct aiVector3D* max,
            struct aiMatrix4x4* trafo
        );
    void loadScene(const QString& filename);
private:
     const struct aiScene* sc;
     struct aiVector3D scene_min, scene_max, scene_center;
};
};
#endif
