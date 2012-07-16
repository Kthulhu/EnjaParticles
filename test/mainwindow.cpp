#include <QtGui>
#include <QtOpenGL>
#include <QComboBox>
#include <QFileDialog>
#include "glwidget.h"
#include "mainwindow.h"
#include "sphparametergroup.h"

namespace rtps
{
 MainWindow::MainWindow(std::string path)
 {
     centralWidget = new QWidget;
     setCentralWidget(centralWidget);
     QGLFormat fmt(QGL::AlphaChannel|QGL::DeprecatedFunctions|QGL::SampleBuffers);
     fmt.setProfile(QGLFormat::CompatibilityProfile);
     QGLContext* ctx = new QGLContext(fmt);

     glWidget = new GLWidget(ctx,path,this);
     glWidget->setFocus();
     glWidget->setFocusPolicy(Qt::WheelFocus);

     systemSelector = new QComboBox(this);
     systemSelector->addItem(QString("Please Load Parameter File"));
     systemSelectorLabel = new QLabel("System:",this);
     connect(glWidget,SIGNAL(systemMapChanged(const std::vector<std::string>&)),this,SLOT(setSystemNames(const std::vector<std::string>&)));

     rendererSelector = new QComboBox(this);
     rendererSelector->addItem(QString("Points"));
     rendererSelector->addItem(QString("Screen Space"));
     rendererSelector->addItem(QString("Mesh Renderer"));
     connect(rendererSelector, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setRenderer(const QString&)));

     connect(this,SIGNAL(rendererChanged(const QString&, const QString&)),glWidget, SLOT(changeRenderer(const QString&, const QString&)));

     sphParams = new SPHParameterGroup(Qt::Horizontal, "SPH Parameters",this);
     connect(sphParams, SIGNAL(valueChanged(const QString&,const QString&)), this, SLOT(valueChanged(const QString&, const QString&)));

     connect(this, SIGNAL(parameterValueChanged(const QString&,const QString&,const QString&)),
             glWidget,SLOT(setParameterValue(const QString&,const QString&,const QString&)));

     createActions();
     createMenus();

     QGridLayout *centralLayout = new QGridLayout;
     //centralLayout->addWidget(glWidgetArea, 0, 0);
     centralLayout->addWidget(systemSelectorLabel, 0, 0);
     centralLayout->addWidget(systemSelector, 0, 1,1,1);
     centralLayout->addWidget(new QLabel("Renderer:"), 1, 0);
     centralLayout->addWidget(rendererSelector, 1, 1,1,1);
     /*centralLayout->addWidget(slider1Label, 1, 0);
     centralLayout->addWidget(slider1, 1, 1 );
     centralLayout->addWidget(slider2Label, 2, 0);
     centralLayout->addWidget(slider2, 2, 1);*/
     centralLayout->addWidget(sphParams,2,0,14,2);
     centralLayout->addWidget(glWidget, 0, 2,15,6);

     centralWidget->setLayout(centralLayout);
     this->setStyleSheet("QMainWindow {background: qlineargradient(x1: 1, y1: 0, x2: 0, y2: 0, stop: 0 #777777, stop: 0.65 #777777, stop: 1 #F9F9F9);}");

     setWindowTitle(tr("RTPS Library Test"));
     resize(1250, 700);
 }

 void MainWindow::about()
 {
     QMessageBox::about(this, tr("About RTPS Library Test"),
             tr("RTPS is a library for simulating fluids and rigid bodies in real-time via particle based methods."));
 }

 void MainWindow::createActions()
 {
    loadParametersAct = new QAction(tr("Load &Parameters..."), this);
    loadParametersAct->setShortcut(tr("Ctrl+P"));
    connect(loadParametersAct, SIGNAL(triggered()),
                                this, SLOT(loadParameters()));

    loadSceneAct = new QAction(tr("Load &Scene..."), this);
    loadSceneAct->setShortcut(tr("Ctrl+S"));
    connect(loadSceneAct, SIGNAL(triggered()),
                                this, SLOT(loadScene()));

    loadDynamicMeshesAct = new QAction(tr("Load Dynamic &Meshes..."), this);
    loadDynamicMeshesAct->setShortcut(tr("Ctrl+M"));
    connect(loadDynamicMeshesAct, SIGNAL(triggered()),
                                this, SLOT(loadDynamicMeshes()));
    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    aboutAct = new QAction(tr("&About"), this);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAct = new QAction(tr("About &Qt"), this);
    connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
 }

 void MainWindow::loadParameters()
 {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"../test",tr("RTPS Params (*.xml)"));
    if(!fileName.isEmpty())
        glWidget->loadParameterFile(fileName);
 }

 void MainWindow::loadScene()
 {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"../test",tr("Wavefront (*.obj)"));
    if(!fileName.isEmpty())
        glWidget->loadScene(fileName);
 }

 void MainWindow::loadDynamicMeshes()
 {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),".",tr("Wavefront (*.obj)"));
    if(!fileName.isEmpty())
        glWidget->loadMeshScene(fileName);
 }

 void MainWindow::createMenus()
 {
     fileMenu = menuBar()->addMenu(tr("&File"));
     fileMenu->addAction(loadParametersAct);
     fileMenu->addAction(loadSceneAct);
     fileMenu->addAction(loadDynamicMeshesAct);
     fileMenu->addSeparator();
     fileMenu->addAction(exitAct);

     helpMenu = menuBar()->addMenu(tr("&Help"));
     helpMenu->addAction(aboutAct);
     helpMenu->addAction(aboutQtAct);
 }
void MainWindow::setSystemNames(const std::vector<std::string>& sysNames)
{
    systemSelector->clear();
    for(int i = 0;i<sysNames.size(); i++)
    {
        systemSelector->addItem(QString(sysNames[i].c_str()));
    }
}

/* QSlider *MainWindow::createSlider(const char *name)
 {
     QSlider *slider = new QSlider(Qt::Horizontal,this);
     slider->setObjectName(name);
     slider->setRange(0, 100);
     slider->setSingleStep(10);
     slider->setPageStep(10);
     slider->setTickInterval(10);
     slider->setTickPosition(QSlider::TicksRight);
     connect(slider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
     return slider;
 }*/
void MainWindow::setRenderer(const QString& renderer)
{
    QString system = systemSelector->currentText();
    emit rendererChanged(system,renderer);
}

void MainWindow::valueChanged(const QString& parameterName, const QString& value)
{
    QString system = systemSelector->currentText();
    /*dout<<"----------"<<endl;
    dout<<"sys = "<<(const char*)system.toAscii().data()<<endl;
    dout<<"parameter = "<<(const char*)parameterName.toAscii().data()<<endl;
    dout<<"value = "<<(const char*)value.toAscii().data()<<endl;
    dout<<"----------"<<endl;*/
    emit parameterValueChanged(system,parameterName,value);
}
}
