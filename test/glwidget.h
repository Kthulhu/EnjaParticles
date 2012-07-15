#ifndef GLWIDGET_H
 #define GLWIDGET_H

#include "../rtpslib/structs.h"
#include "../rtpslib/render/ParticleEffect.h"
#include "../rtpslib/system/ParticleShape.h"

#include <map>
#include <string>
#include <fstream>
#include <QGLWidget>
#include <QElapsedTimer>
#include <QString>
#include <QTimer>
//#include <QtGui>


namespace rtps
{
    struct Mesh;

    class System;
    class AIWrapper;
    class MeshEffect;
    class ParticleEffect;
    class ShaderLibrary;
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
        void ResetSimulations();
        void changeRenderer(const QString& system, const QString& renderer);

    signals:
        void systemMapChanged(const std::vector<std::string>& sysNames);
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
        ParticleShape* createParticleShape(const QString& system, Mesh* mesh);

    protected slots:
        void update();
    private:
        QElapsedTimer* elapsedTimer;
        GLuint environTex;
        std::map<QString,System*> systems;
        std::map<QString,QString> systemRenderType;
        std::map<QString,ParticleEffect*> effects;
        MeshEffect* meshRenderer;
        std::map<QString,ParticleShape*> pShapes;
        std::map<QString,Mesh*> meshes;
        std::map<QString,Mesh*> dynamicMeshes;
        ShaderLibrary* lib;

        Camera* view;
        Light* light;
        int2 mousePos;
        int mouseButtons;

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
        std::string binaryPath;
    };
};
 #endif
