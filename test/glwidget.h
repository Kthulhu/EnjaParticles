#ifndef GLWIDGET_H
 #define GLWIDGET_H


#include "../rtpslib/structs.h"

#include <map>
#include <string>
#include <fstream>
#include <QGLWidget>
#include <QElapsedTimer>
#include <QString>
#include <QTimer>
//#include <QtGui>

//class QGLWidget;
class QString;
class QTimer;

namespace rtps
{
    struct Mesh;
	struct Light;
	struct Material;

	class Camera;
    class System;
    class AIWrapper;
    class MeshEffect;
    class ParticleEffect;
    class StreamlineEffect;
    class ParticleShape;
    class ShaderLibrary;
    class RTPSSettings;
    class CL;


    class GLWidget : public QGLWidget
    {
        Q_OBJECT

    public:
        GLWidget(QGLContext* ctx,std::string bpath, QWidget *parent = 0);
        ~GLWidget();


    public slots:
        void setParameterValue(const QString& system, const QString& parameter, const QString& value);
        void loadScene(const QString& filename);
        void loadMeshScene(const QString& filename);
        void loadParameterFile(const QString& filename);
        void getSystemSettings(const QString& system);
        void ResetSimulations();
        void changeRenderer(const QString& system, const QString& renderer);
        void addRigidBody(const QString& system, const QString& mesh, float4 pos, float4 vel, float mass);

    signals:
        void systemMapChanged(const std::vector<std::string>& sysNames);
        void initRendererPanel(const QString& renderer, RTPSSettings* settings);
        void initSystemPanel(const QString& systemType, RTPSSettings* settings);
        void meshListUpdated(const std::vector<QString>& meshNames);

        //void parameterValueChanged(const QString& parameter, const QString& value);

    protected:
        void renderSkyBox();
        void cameraChanged();
        void initializeGL();
        void paintGL();
        void resizeGL(int width, int height);
        void mousePressEvent(QMouseEvent *event);
        void mouseReleaseEvent(QMouseEvent *event);
        void mouseMoveEvent(QMouseEvent *event);
        void keyPressEvent(QKeyEvent *event);
        void readParamFile(std::istream& is);
        int writeMovieFrame(const char* filename, const char* dir);
        void display(bool transparent);
        void createSceneTextures();
        ParticleShape* createParticleShape(const QString& system, Mesh* mesh);

    protected slots:
        void update();
    private:
        QElapsedTimer* elapsedTimer;
        GLuint environTex;
        GLuint sceneFBO;
        GLuint sceneTex[4]; ///store scene color buffer and scene depth buffer need quad buffer for stereo
        std::map<QString,System*> systems;
        std::map<QString,QString> systemRenderType;
        std::map<QString,ParticleEffect*> effects;
        MeshEffect* meshRenderer;
        StreamlineEffect* streamlineRenderer;
        std::map<QString,ParticleShape*> pShapes;
        std::map<std::string,Mesh* > meshes;
        std::map<std::string,Mesh* > dynamicMeshes;
        ShaderLibrary* lib;

        Camera* view;
        Light* light;
        int2 mousePos;
        int mouseButtons;
        bool stereoscopic;

        AIWrapper* scene;
        AIWrapper* dynamicMeshScene;
        //GLuint scene_list;
        GLuint skyboxVBO,skyboxTexVBO;
        std::string currentMesh;

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
