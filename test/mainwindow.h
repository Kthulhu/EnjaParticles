#ifndef MAINWINDOW_H
 #define MAINWINDOW_H

 #include <QMainWindow>
#include <string.h>

 class QAction;
 class QLabel;
 class QMenu;
 class QScrollArea;
 class QSlider;
 class QComboBox;
 class QString;
 class GLWidget;
 class SPHParameterGroup;
namespace rtps
{
 class MainWindow : public QMainWindow
 {
     Q_OBJECT

 public:
     MainWindow(std::string path);

signals:
void parameterValueChanged(const QString& system, const QString& parameterName, const QString& value);
void rendererChanged(const QString& system, const QString& renderer);


 private slots:
     void about();
     void loadParameters();
     void loadScene();
     void loadDynamicMeshes();
     void setSystemNames(const std::vector<std::string>& sysNames);
     void setRenderer(const QString& value);
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
