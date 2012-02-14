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
//FIXME: This should eventually be part of the test framework. I Just
//added it in global space in order to simplify testing the model loading.
// ----------------------------------------------------------------------------
void get_bounding_box_for_node (const struct aiNode* nd,
	struct aiVector3D* min,
	struct aiVector3D* max,
	struct aiMatrix4x4* trafo
){
	struct aiMatrix4x4 prev;
	unsigned int n = 0, t;

	prev = *trafo;
	aiMultiplyMatrix4(trafo,&nd->mTransformation);

	for (; n < nd->mNumMeshes; ++n) {
		const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
		for (t = 0; t < mesh->mNumVertices; ++t) {

			struct aiVector3D tmp = mesh->mVertices[t];
			aiTransformVecByMatrix4(&tmp,trafo);

			min->x = aisgl_min(min->x,tmp.x);
			min->y = aisgl_min(min->y,tmp.y);
			min->z = aisgl_min(min->z,tmp.z);

			max->x = aisgl_max(max->x,tmp.x);
			max->y = aisgl_max(max->y,tmp.y);
			max->z = aisgl_max(max->z,tmp.z);
		}
	}

	for (n = 0; n < nd->mNumChildren; ++n) {
		get_bounding_box_for_node(nd->mChildren[n],min,max,trafo);
	}
	*trafo = prev;
}

// ----------------------------------------------------------------------------
void get_bounding_box (struct aiVector3D* min, struct aiVector3D* max)
{
	struct aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);

	min->x = min->y = min->z =  1e10f;
	max->x = max->y = max->z = -1e10f;
	get_bounding_box_for_node(scene->mRootNode,min,max,&trafo);
}

// ----------------------------------------------------------------------------
void color4_to_float4(const struct aiColor4D *c, float f[4])
{
	f[0] = c->r;
	f[1] = c->g;
	f[2] = c->b;
	f[3] = c->a;
}

// ----------------------------------------------------------------------------
void set_float4(float f[4], float a, float b, float c, float d)
{
	f[0] = a;
	f[1] = b;
	f[2] = c;
	f[3] = d;
}

// ----------------------------------------------------------------------------
void apply_material(const struct aiMaterial *mtl)
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
	unsigned int max;

    max = 1;
    aiGetMaterialFloatArray(mtl,AI_MATKEY_OPACITY,&opacity,&max);
    dout<<"Opacity: "<< opacity<<" Max "<<max<<std::endl;
	set_float4(c, 0.8f, 0.8f, 0.8f, 1.0f);
	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
		color4_to_float4(&diffuse, c);
    c[3]=opacity;
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, c);

    dout<<"red "<<c[0]<<" green  "<<c[1]<<" blue  "<<c[2]<<"  alpha  "<<c[3]<<std::endl;
	set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
		color4_to_float4(&specular, c);
    c[3]=opacity;
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);

    dout<<"red "<<c[0]<<" green  "<<c[1]<<" blue  "<<c[2]<<"  alpha  "<<c[3]<<std::endl;
	set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
		color4_to_float4(&ambient, c);
    c[3]=opacity;
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, c);

    dout<<"red "<<c[0]<<" green  "<<c[1]<<" blue  "<<c[2]<<"  alpha  "<<c[3]<<std::endl;
	set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
	if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
		color4_to_float4(&emission, c);
    c[3]=opacity;
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, c);

    dout<<"red "<<c[0]<<" green  "<<c[1]<<" blue  "<<c[2]<<"  alpha  "<<c[3]<<std::endl;
	max = 1;
	ret1 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
	if(ret1 == AI_SUCCESS) {
    	max = 1;
    	ret2 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &strength, &max);
		if(ret2 == AI_SUCCESS)
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess * strength);
        else
        	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    }
	else {
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
		set_float4(c, 0.0f, 0.0f, 0.0f, 0.0f);
        c[3]=opacity;
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);
	}

	max = 1;
	if(AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_ENABLE_WIREFRAME, &wireframe, &max))
		fill_mode = wireframe ? GL_LINE : GL_FILL;
	else
		fill_mode = GL_FILL;
	glPolygonMode(GL_FRONT_AND_BACK, fill_mode);

	max = 1;
	if((AI_SUCCESS == aiGetMaterialIntegerArray(mtl, AI_MATKEY_TWOSIDED, &two_sided, &max)) && two_sided)
		glDisable(GL_CULL_FACE);
	else
		glEnable(GL_CULL_FACE);
}

// ----------------------------------------------------------------------------
void recursive_render (const struct aiScene *sc, const struct aiNode* nd)
{
	int i;
	unsigned int n = 0, t;
	struct aiMatrix4x4 m = nd->mTransformation;

	// update transform
	aiTransposeMatrix4(&m);
	glPushMatrix();
	glMultMatrixf((float*)&m);

	// draw all meshes assigned to this node
	for (; n < nd->mNumMeshes; ++n) {
		const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];

		apply_material(sc->mMaterials[mesh->mMaterialIndex]);

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
			}

			glEnd();
		}

	}

	// draw all children
	for (n = 0; n < nd->mNumChildren; ++n) {
		recursive_render(sc, nd->mChildren[n]);
	}

	glPopMatrix();
}
void display(void)
{
	float tmp;
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);    // Uses default lighting parameters

	glEnable(GL_DEPTH_TEST);

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glEnable(GL_NORMALIZE);

	// XXX docs say all polygons are emitted CCW, but tests show that some aren't.
	if(getenv("MODEL_IS_BROKEN"))
		glFrontFace(GL_CW);

	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);


	if(scene_list == 0) {
	    scene_list = glGenLists(1);
	    glNewList(scene_list, GL_COMPILE);
            // now begin at the root node of the imported data and traverse
            // the scenegraph by multiplying subsequent local transforms
            // together on GL's matrix stack.
	    recursive_render(scene, scene->mRootNode);
	    glEndList();
	}

	glCallList(scene_list);
    glDisable(GL_LIGHTING);
	glDisable(GL_NORMALIZE);
}
        private:
            GLuint windowWidth,windowHeight;
            std::map<std::string,System*> systems;
            std::map<std::string,ParticleEffect*> effects;
            std::map<std::string,ParticleShape*> pShapes;
            std::map<std::string,GLuint> meshVBOs;
            std::map<std::string,GLuint> meshIBOs;
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
