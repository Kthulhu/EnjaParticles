#ifndef MAINWINDOW_H
 #define MAINWINDOW_H

 #include <QMainWindow>
#include <string.h>
#include <map>
#include "../rtpslib/structs.h"

 class QAction;
 class QLabel;
 class QMenu;
 class QScrollArea;
 class QSlider;
 class QComboBox;
 class QString;
 class QStackedWidget;

namespace rtps
{
class GLWidget;
class SPHParameterGroup;
class RigidbodyParameterGroup;
class FlockingParameterGroup;
class ParticleEffectParameterGroup;
class RTPSSettings;
 class MainWindow : public QMainWindow
 {
     Q_OBJECT

 public:
     MainWindow(std::string path);

signals:
void parameterValueChanged(const QString& system, const QString& parameterName, const QString& value);
void rendererChanged(const QString& system, const QString& renderer);
void getSystemSettings(const QString& system);
void addRigidBody(const QString& system, const QString& mesh, float4 pos, float4 vel, float mass);

 public slots:
void initRendererPanel(const QString& renderer, RTPSSettings* settings);
void initSystemPanel(const QString& systemType, RTPSSettings* settings);
void addRigidBody(const QString& mesh, float4 pos, float4 vel, float mass);

 private slots:
     void about();
     void loadParameters();
     void loadScene();
     void loadDynamicMeshes();
     void setSystemNames(const std::vector<std::string>& sysNames);
     void setRenderer(const QString& value);
     void setSystem(const QString& system);
     void valueChanged(const QString& parameterName, const QString& value);

 private:
     void createActions();
     void createMenus();
     //QSlider *createSlider(const char *changedSignal, const char *setterSlot);
     //
     QSlider *createSlider(const char *name);

     QWidget *centralWidget;
     QScrollArea *parameters;
     GLWidget *glWidget;
     QComboBox *systemSelector;
     QLabel *systemSelectorLabel;
     QComboBox *rendererSelector;

     SPHParameterGroup *sphParams;
     RigidbodyParameterGroup *rbParams;
     FlockingParameterGroup *flockingParams;
     QStackedWidget *systemParamPanels;
     std::map<QString, unsigned int> systemParamPanelID;
     ParticleEffectParameterGroup *ssEffectParams;
     QStackedWidget *effectParamPanels;
     std::map<QString, unsigned int> effectParamPanelID;

     QMenu *fileMenu;
     QMenu *helpMenu;
     QAction *loadParametersAct;
     QAction *loadSceneAct;
     QAction *loadDynamicMeshesAct;
     QAction *exitAct;
     QAction *aboutAct;
     QAction *aboutQtAct;
 };
}
 #endif
