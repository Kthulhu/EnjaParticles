#ifndef MAINWINDOW_H
 #define MAINWINDOW_H

 #include <QMainWindow>
#include <string.h>

 class QAction;
 class QLabel;
 class QMenu;
 class QScrollArea;
 class QSlider;

 class GLWidget;
namespace rtps
{
 class MainWindow : public QMainWindow
 {
     Q_OBJECT

 public:
     MainWindow(std::string path);

 private slots:
     void renderIntoPixmap();
     void grabFrameBuffer();
     void clearPixmap();
     void about();

 private:
     void createActions();
     void createMenus();
     QSlider *createSlider(const char *changedSignal, const char *setterSlot);
     void setPixmap(const QPixmap &pixmap);
     QSize getSize();

     QWidget *centralWidget;
     QScrollArea *glWidgetArea;
     QScrollArea *pixmapLabelArea;
     GLWidget *glWidget;
     QLabel *pixmapLabel;
     QSlider *xSlider;
     QSlider *ySlider;
     QSlider *zSlider;

     QMenu *fileMenu;
     QMenu *helpMenu;
     QAction *grabFrameBufferAct;
     QAction *renderIntoPixmapAct;
     QAction *clearPixmapAct;
     QAction *exitAct;
     QAction *aboutAct;
     QAction *aboutQtAct;
 };
};
 #endif
