#ifndef GLWIDGET_H
 #define GLWIDGET_H

#include <QGLWidget>
#include <QString>
#include <map>
#include <string>
#include <iostream>
#include <fstream>

#include "../../rtpslib/render/MeshEffect.h"
#include "../../rtpslib/system/System.h"
#include "../../rtpslib/system/ParticleShape.h"
#include "../../rtpslib/render/ParticleEffect.h"

#include "../../rtpslib/render/util/stb_image_write.h"
#include "../aiwrapper.h"
namespace rtps
{
 class GLWidget : public QGLWidget
 {
     Q_OBJECT

 public:
     GLWidget(std::string bpath, QWidget *parent = 0);
     ~GLWidget();


 public slots:
     void setParameterValue(const QString& system, const QString& parameter, const string& value);
     void loadScene(const QString& filename);
     void loadMeshScene(const QString& filename);
     void loadParameterFile(const QString& filename);
     void ResetSimulations();

 signals:
     //void parameterValueChanged(const QString& parameter, const QString& value);

 protected:
     void initializeGL();
     void paintGL();
     void resizeGL(int width, int height);
     void mousePressEvent(QMouseEvent *event);
     void mouseMoveEvent(QMouseEvent *event);
     void keyPressEvent(QKeyEvent *event);
     void readParamFile(std::istream& is);
     int writeMovieFrame(const char* filename, const char* dir);
     void display(bool transparent);
     ParticleShape* createParticleShape(const QString& system, Mesh* mesh);

 private:
	GLuint environTex;
	std::map<QString,System*> systems;
	std::map<QString,QString> systemRenderType;
	std::map<QString,ParticleEffect*> effects;
	MeshEffect* meshRenderer;
	std::map<QString,ParticleShape*> pShapes;
	std::map<QString,Mesh*> meshes;
	std::map<QString,Mesh*> dynamicMeshes;
	ShaderLibrary* lib;
	//QString renderType;
	float4 rotation; //may want to consider quaternions for this at some point.
	float3 translation;
	Light light;
	int2 mousePos;
	float fov,near,far;
	int mouseButtons;
	// the global Assimp scene object
	AIWrapper* scene;
	AIWrapper* dynamicMeshScene;
	GLuint scene_list;
	
	int frameCounter;
	bool renderMovie;

	float4 gridMax,gridMin;
	float sizeScale;
	float mass;
	CL* cli;
	bool paused,renderVelocity;
	string binaryPath;
 };
};
 #endif
