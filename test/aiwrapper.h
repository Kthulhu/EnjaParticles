#include <assimp/assimp.h>
#include <assimp/aiPostProcess.h>
#include <assimp/aiScene.h>


class AIWrapper
{
public:
    void build_shapes (const struct aiScene *sc, const struct aiNode* nd, struct aiMatrix4x4 parentTransform=aiMatrix4x4(1.0f,0.0f,0.0f,0.0f,
                                                                                                                    0.0f,1.0f,0.0f,0.0f,
                                                                                                                    0.0f,0.0f,1.0f,0.0f,
                                                                                                                    0.0f,0.0f,0.0f,1.0f) );
    void recursive_render (const struct aiScene *sc, const struct aiNode* nd);
    void apply_material(const struct aiMaterial *mtl, Mesh* mesh);
    void set_float4(float f[4], float a, float b, float c, float d);
    void color4_to_float4(const struct aiColor4D *c, float f[4]);
    void get_bounding_box (struct aiVector3D* min, struct aiVector3D* max);
    void get_bounding_box_for_node (const struct aiNode* nd,
            struct aiVector3D* min,
            struct aiVector3D* max,
            struct aiMatrix4x4* trafo
        );
private:
     const struct aiScene* scene;
     struct aiVector3D scene_min, scene_max, scene_center;
};
