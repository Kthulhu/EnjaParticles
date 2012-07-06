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
 class GLWidget;
 class SPHParameterGroup;
namespace rtps
{
 class MainWindow : public QMainWindow
 {
     Q_OBJECT

 public:
     MainWindow(std::string path);

 private slots:
     void about();
     void loadParameters();
     void loadScene();
     void loadDynamicMeshes();
     void setSystemNames(const std::vector<std::string>& sysNames);
     void valueChanged(const QString& parameterName, const QString& value);

 private:
     void createActions();
     void createMenus();
     //QSlider *createSlider(const char *changedSignal, const char *setterSlot);
     //
     QSlider *createSlider(const char *name);
     QSize getSize();

     QWidget *centralWidget;
     QScrollArea *parameters;
     GLWidget *glWidget;
     QComboBox *systemSelector;
     QLabel *systemSelectorLabel;

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
};
 #endif
