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

 private:
     void createActions();
     void createMenus();
     QSlider *createSlider(const char *changedSignal, const char *setterSlot);
     void setPixmap(const QPixmap &pixmap);
     QSize getSize();

     QWidget *centralWidget;
     GLWidget *glWidget;
     QComboBox *systemSelector;

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
