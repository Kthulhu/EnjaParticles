#include <GL/glew.h>
#include "aiwrapper.h"
#include <sstream>
//#include "../rtpslib/RTPS.h"
using namespace std;
namespace rtps
{
const struct aiScene* AIWrapper::getScene()
{
	return sc;
}
void AIWrapper::loadMeshes (std::map<QString,Mesh*>& meshes, const struct aiNode* nd, struct aiMatrix4x4 parentTransform)
{
unsigned int n = 0, t,i;

        struct aiMatrix4x4 m = nd->mTransformation;
        aiMultiplyMatrix4(&m,&parentTransform);

        // update transform
        aiTransposeMatrix4(&m);

        float16 mat;
        memcpy(&mat,&m,sizeof(float16));
        //mat.print("aiMatrix");

        for (; n < nd->mNumMeshes; ++n) {
            const struct aiMesh* mesh = sc->mMeshes[nd->mMeshes[n]];

            //dout<<"num faces "<<mesh->mNumFaces<<endl;

            Mesh* me=new Mesh();
            //dout<<"material index = "<<mesh->mMaterialIndex<<endl;
            apply_material(sc->mMaterials[mesh->mMaterialIndex],me);
            unsigned int* ibo = new unsigned int[mesh->mNumFaces*3];
            float* vbo = new float[mesh->mNumVertices*3];
            float* normals=NULL;
            if(mesh->HasNormals())
            {
                normals = new float[mesh->mNumVertices*3];
                me->hasNormals=true;
            }
            float* texcoords=NULL;
            if(mesh->HasTextureCoords(0))
            {
                texcoords = new float[mesh->mNumVertices*2];
                me->hasTexture=true;
            }
            for (t = 0; t < mesh->mNumFaces; ++t) {
                const struct aiFace* face = &mesh->mFaces[t];
                for(i = 0; i < face->mNumIndices; i++) {
                    unsigned int index = face->mIndices[i];
                    ibo[t*3+i]=index;
                    float x=mesh->mVertices[index].x;
                    float y=mesh->mVertices[index].y;
                    float z=mesh->mVertices[index].z;
                    vbo[index*3]=x;
                    vbo[index*3+1]=y;
                    vbo[index*3+2]=z;
                    if(normals)
                    {
                        normals[index*3]=mesh->mNormals[index].x;
                        normals[index*3+1]=mesh->mNormals[index].y;
                        normals[index*3+2]=mesh->mNormals[index].z;
                    }
                    if(texcoords)
                    {
                        texcoords[index*2]=mesh->mTextureCoords[0][index].x;
                        texcoords[index*2+1]=mesh->mTextureCoords[0][index].y;
                    }
            //        dout<<"index = "<<index<<"x = "<<x<<" "<<"y = "<<y<<" "<<"z = "<<z<<endl;
                    /*if(x<minCoord.x)
                        minCoord.x=x;
                    if(x>maxCoord.x)
                        maxCoord.x=x;
                    if(y<minCoord.y)
                        minCoord.y=y;
                    if(y>maxCoord.y)
                        maxCoord.y=y;
                    if(z<minCoord.z)
                        minCoord.z=z;
                    if(z>maxCoord.z)
                        maxCoord.z=z;*/
                }
            }
            me->modelMat=mat;
            me->vbo=createVBO(vbo,mesh->mNumVertices*3*sizeof(float),GL_ARRAY_BUFFER,GL_STATIC_DRAW );
            me->vboSize=mesh->mNumVertices;
            delete[] vbo;
            me->ibo=createVBO(ibo, mesh->mNumFaces*3*sizeof(int),GL_ELEMENT_ARRAY_BUFFER,GL_STATIC_DRAW );
            me->iboSize=mesh->mNumFaces*3;
            delete[] ibo;
            if(normals)
            {
                me->normalbo=createVBO(normals,mesh->mNumVertices*3*sizeof(float),GL_ARRAY_BUFFER,GL_STATIC_DRAW );
            }
            if(texcoords)
            {
                me->texCoordsbo=createVBO(texcoords,mesh->mNumVertices*2*sizeof(float),GL_ARRAY_BUFFER,GL_STATIC_DRAW );
            }

            stringstream s;
            s<<mesh->mName.data<<mesh->mNumFaces;
            dout<<"Mesh name = "<<s.str()<<endl;
            meshes[QString(s.str().c_str())]=me;
            //dout<<"minCoord ("<<minCoord.x<<","<<minCoord.y<<","<<minCoord.z<<")"<<endl;
            //dout<<"maxCoord ("<<maxCoord.x<<","<<maxCoord.y<<","<<maxCoord.z<<")"<<endl;
            //Add padding equalt to spacing to ensure that all of the mesh is voxelized.
            /*float space = systems["rb1"]->getSpacing()/2.f;
            float3 adjminCoord=float3(minCoord.x-space,minCoord.y-space,minCoord.z-space);
            float3 adjmaxCoord=float3(maxCoord.x+space,maxCoord.y+space,maxCoord.z+space);
            space = systems["rb1"]->getSpacing();
            ParticleShape* shape = new ParticleShape(adjminCoord,adjmaxCoord,space);*/
        }

        // draw all children
        for (n = 0; n < nd->mNumChildren; ++n) {
            loadMeshes(meshes, nd->mChildren[n],m);
        }
}
/*void AIWrapper::recursive_render (const struct aiNode* nd)
{
unsigned int n = 0, t,i=0;
        struct aiMatrix4x4 m = nd->mTransformation;

        // update transform
        aiTransposeMatrix4(&m);
        glPushMatrix();
        glMultMatrixf((float*)&m);

        // draw all meshes assigned to this node
        for (; n < nd->mNumMeshes; ++n) {
            const struct aiMesh* mesh = sc->mMeshes[nd->mMeshes[n]];

            //apply_material(sc->mMaterials[mesh->mMaterialIndex]);

            if(mesh->mNormals == NULL) {
                glDisable(GL_LIGHTING);
            } else {
                glEnable(GL_LIGHTING);
            }


            for (t = 0; t < mesh->mNumFaces; ++t) {
                const struct aiFace* face = &mesh->mFaces[t];
                GLenum face_mode;

                switch(face->mNumIndices) {
                    case 1: face_mode = GL_POINTS; break;
                    case 2: face_mode = GL_LINES; break;
                    case 3: face_mode = GL_TRIANGLES; break;
                    default: face_mode = GL_POLYGON; break;
                }

                //dout<<"Face mode = "<<face_mode<<" GL_TRIANGLES = "<<GL_TRIANGLES<<endl;
                glBegin(face_mode);


                for(i = 0; i < face->mNumIndices; i++) {
                    int index = face->mIndices[i];
                    if(mesh->mColors[0] != NULL)
                    {
                        glColor4fv((GLfloat*)&mesh->mColors[0][index]);
                    }
                    if(mesh->mNormals != NULL)
                        glNormal3fv(&mesh->mNormals[index].x);
                    glVertex3fv(&mesh->mVertices[index].x);
                    //dout<<"index = "<<index<<"x = "<<mesh->mVertices[index].x<<" "<<"y = "<<mesh->mVertices[index].y<<" "<<"z = "<<mesh->mVertices[index].z<<endl;
                }

                glEnd();
            }

        }

        // draw all children
        for (n = 0; n < nd->mNumChildren; ++n) {
            recursive_render(nd->mChildren[n]);
        }

        glPopMatrix();
}*/
void AIWrapper::apply_material(const struct aiMaterial *mtl, Mesh* mesh)
{
float c[4];

        GLenum fill_mode;
        int ret1, ret2;
        struct aiColor4D diffuse;
        struct aiColor4D specular;
        struct aiColor4D ambient;
        struct aiColor4D emission;
        float shininess, strength, opacity;
        int two_sided;
        int wireframe;
        unsigned int maxVal;
	if(!mtl)
		cerr<<"No Material Found"<<endl;

        maxVal = 1;
        aiGetMaterialFloatArray(mtl,AI_MATKEY_OPACITY,&opacity,&maxVal);
        mesh->material.opacity = opacity;
        dout<<"Opacity: "<< opacity<<" Max "<<maxVal<<std::endl;
        set_float4(c, 0.8f, 0.8f, 0.8f, 1.0f);
        if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
            color4_to_float4(&diffuse, c);
        c[3]=opacity;
        memcpy(&mesh->material.diffuse.x,c,sizeof(float3));
        //glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, c);

        dout<<"red "<<c[0]<<" green  "<<c[1]<<" blue  "<<c[2]<<"  alpha  "<<c[3]<<std::endl;
        set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
        if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
            color4_to_float4(&specular, c);
        c[3]=opacity;
        memcpy(&mesh->material.specular.x,c,sizeof(float3));
        //glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);

        dout<<"red "<<c[0]<<" green  "<<c[1]<<" blue  "<<c[2]<<"  alpha  "<<c[3]<<std::endl;
        set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
        if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
            color4_to_float4(&ambient, c);
        c[3]=opacity;
        memcpy(&mesh->material.ambient.x,c,sizeof(float3));
        //glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, c);

        dout<<"red "<<c[0]<<" green  "<<c[1]<<" blue  "<<c[2]<<"  alpha  "<<c[3]<<std::endl;
        set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
        if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
            color4_to_float4(&emission, c);
        c[3]=opacity;
        //glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, c);

        dout<<"red "<<c[0]<<" green  "<<c[1]<<" blue  "<<c[2]<<"  alpha  "<<c[3]<<std::endl;
        maxVal = 1;
        ret1 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &maxVal);
        if(ret1 == AI_SUCCESS) {
            maxVal = 1;
            ret2 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &strength, &maxVal);
            if(ret2 == AI_SUCCESS)
            {
                //glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess * strength);
                mesh->material.shininess=shininess*strength;
            }
            else
            {
                //glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
                mesh->material.shininess=shininess;
            }
        }
        else {
            mesh->material.shininess=0.0f;
           // glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
            //set_float4(c, 0.0f, 0.0f, 0.0f, 0.0f);
            //c[3]=opacity;
            //glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);
        }

        maxVal = 1;
        if(AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_ENABLE_WIREFRAME, &wireframe, &maxVal))
            fill_mode = wireframe ? GL_LINE : GL_FILL;
        else
            fill_mode = GL_FILL;
        //glPolygonMode(GL_FRONT_AND_BACK, fill_mode);
}
void AIWrapper::set_float4(float f[4], float a, float b, float c, float d)
{
        f[0] = a;
        f[1] = b;
        f[2] = c;
        f[3] = d;
}
void AIWrapper::color4_to_float4(const struct aiColor4D *c, float f[4])
{
        f[0] = c->r;
        f[1] = c->g;
        f[2] = c->b;
        f[3] = c->a;
}
void AIWrapper::get_bounding_box (struct aiVector3D* minBB, struct aiVector3D* maxBB)
{
        struct aiMatrix4x4 trafo;
        aiIdentityMatrix4(&trafo);

        minBB->x = minBB->y = minBB->z =  1e10f;
        maxBB->x = maxBB->y = maxBB->z = -1e10f;
        get_bounding_box_for_node(sc->mRootNode,minBB,maxBB,&trafo);
}
void AIWrapper::get_bounding_box_for_node (const struct aiNode* nd,
    struct aiVector3D* minBB,
    struct aiVector3D* maxBB,
    struct aiMatrix4x4* trafo
)
{
        struct aiMatrix4x4 prev;
        unsigned int n = 0, t;

        prev = *trafo;
        aiMultiplyMatrix4(trafo,&nd->mTransformation);

        for (; n < nd->mNumMeshes; ++n) {
            const struct aiMesh* mesh = sc->mMeshes[nd->mMeshes[n]];
            for (t = 0; t < mesh->mNumVertices; ++t) {

                struct aiVector3D tmp = mesh->mVertices[t];
                aiTransformVecByMatrix4(&tmp,trafo);

                minBB->x = aisgl_min(minBB->x,tmp.x);
                minBB->y = aisgl_min(minBB->y,tmp.y);
                minBB->z = aisgl_min(minBB->z,tmp.z);

                maxBB->x = aisgl_max(maxBB->x,tmp.x);
                maxBB->y = aisgl_max(maxBB->y,tmp.y);
                maxBB->z = aisgl_max(maxBB->z,tmp.z);
            }
        }

        for (n = 0; n < nd->mNumChildren; ++n) {
            get_bounding_box_for_node(nd->mChildren[n],minBB,maxBB,trafo);
        }
        *trafo = prev;
}
void AIWrapper::loadScene(const QString& filename)
{
        // we are taking one of the postprocessing presets to avoid
        // spelling out 20+ single postprocessing flags here.
        sc = aiImportFile(filename.toAscii().data(),aiProcessPreset_TargetRealtime_MaxQuality);

        if (sc) {
            get_bounding_box(&scene_min,&scene_max);
            scene_center.x = (scene_min.x + scene_max.x) / 2.0f;
            scene_center.y = (scene_min.y + scene_max.y) / 2.0f;
            scene_center.z = (scene_min.z + scene_max.z) / 2.0f;
        }
        else
        {
            cerr<<"Scene file couldn't be imported from: "<<filename.toAscii().data()<<endl;
        }

}
}
