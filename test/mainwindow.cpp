#include <GL/glew.h>
#include "glwidget.h"
#include "mainwindow.h"
#include <QtGui>
#include <QComboBox>
#include <QFileDialog>
#include <QStackedWidget>
#include "rtpsparametergroup.h"
#include "sphparametergroup.h"
#include "rigidbodyparametergroup.h"
#include "flockingparametergroup.h"
#include "particleeffectparametergroup.h"


namespace rtps
{
 MainWindow::MainWindow(std::string path)
 {
     centralWidget = new QWidget;
     setCentralWidget(centralWidget);
     //QGLFormat fmt(QGL::AlphaChannel|QGL::DeprecatedFunctions|QGL::SampleBuffers);
     //fmt.setProfile(QGLFormat::CompatibilityProfile);
     //QGLFormat fmt(QGL::AlphaChannel|QGL::SampleBuffers);
     QGLFormat fmt(QGL::AlphaChannel);
     fmt.setProfile(QGLFormat::CoreProfile);
     QGLContext* ctx = new QGLContext(fmt);

	 //ctx->makeCurrent();
     glWidget = new GLWidget(ctx,path,this);
     glWidget->setFocus();
     glWidget->setFocusPolicy(Qt::WheelFocus);


     systemSelector = new QComboBox(this);
     systemSelector->addItem(QString("Please Load Parameter File"));
     connect(systemSelector, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setSystem(const QString&)));
     systemSelectorLabel = new QLabel("System:",this);
     connect(glWidget,SIGNAL(systemMapChanged(const std::vector<std::string>&)),this,SLOT(setSystemNames(const std::vector<std::string>&)));

     rendererSelector = new QComboBox(this);
     rendererSelector->addItem(QString("Points"));
     rendererSelector->addItem(QString("Screen Space"));
     rendererSelector->addItem(QString("Mesh Renderer"));
     connect(rendererSelector, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setRenderer(const QString&)));

     connect(this,SIGNAL(rendererChanged(const QString&, const QString&)),glWidget, SLOT(changeRenderer(const QString&, const QString&)));

     connect(this,SIGNAL(getSystemSettings(const QString&)),glWidget,SLOT(getSystemSettings(const QString&)));

     connect(glWidget,SIGNAL(initRendererPanel(const QString&, RTPSSettings*)),
             this,SLOT(initRendererPanel(const QString&, RTPSSettings*)));
     connect(glWidget,SIGNAL(initSystemPanel(const QString&, RTPSSettings*)),
             this,SLOT(initSystemPanel(const QString&, RTPSSettings*)));

     sphParams = new SPHParameterGroup(Qt::Horizontal, "SPH Parameters",this);
     rbParams = new RigidbodyParameterGroup(Qt::Horizontal, "Rigidbody Parameters",this);
     flockingParams = new FlockingParameterGroup(Qt::Horizontal, "Flocking Parameters",this);
     connect(sphParams, SIGNAL(valueChanged(const QString&,const QString&)), this, SLOT(valueChanged(const QString&, const QString&)));
     connect(rbParams, SIGNAL(valueChanged(const QString&,const QString&)), this, SLOT(valueChanged(const QString&, const QString&)));
     connect(flockingParams, SIGNAL(valueChanged(const QString&,const QString&)), this, SLOT(valueChanged(const QString&, const QString&)));
     ssEffectParams = new ParticleEffectParameterGroup(Qt::Horizontal, "Screen Space Parameters",this);
     connect(ssEffectParams, SIGNAL(valueChanged(const QString&,const QString&)), this, SLOT(valueChanged(const QString&, const QString&)));

     connect(glWidget,SIGNAL(meshListUpdated(const std::vector<QString>&)),rbParams,SLOT(meshListUpdated(const std::vector<QString>&)));
     connect(rbParams,SIGNAL(addRigidBody(QString,float4,float4,float)),this,SLOT(addRigidBody(QString,float4,float4,float)));
     connect(this,SIGNAL(addRigidBody(QString,QString,float4,float4,float)),glWidget,SLOT(addRigidBody(QString,QString,float4,float4,float)));
     systemParamPanels = new QStackedWidget(this);
     systemParamPanels->addWidget(sphParams);
     systemParamPanels->addWidget(rbParams);
     systemParamPanels->addWidget(flockingParams);
     effectParamPanels = new QStackedWidget(this);
     effectParamPanels->addWidget(ssEffectParams);

     effectParamPanelID["Points"]=0;
     effectParamPanelID["Screen Space"]=0;
     effectParamPanelID["Mesh Renderer"]=0;
     systemParamPanelID["sph"]=0;
     systemParamPanelID["rigidbody"]=1;
     systemParamPanelID["flock"]=2;

     connect(this, SIGNAL(parameterValueChanged(const QString&,const QString&,const QString&)),
             glWidget,SLOT(setParameterValue(const QString&,const QString&,const QString&)));

     createActions();
     createMenus();
     QScrollArea* scrollArea = new QScrollArea(this);
     scrollArea->setWidgetResizable(true);
     scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
     QWidget* paramWidget = new QWidget;
     paramWidget->setObjectName("paramWidget");

     QVBoxLayout *scrollLayout=new QVBoxLayout;
     scrollLayout->addWidget(systemSelectorLabel);
     scrollLayout->addWidget(systemSelector);
     //scrollLayout->addWidget(sphParams);
     scrollLayout->addWidget(systemParamPanels);
     scrollLayout->addWidget(new QLabel("Renderer:"));
     scrollLayout->addWidget(rendererSelector);
     scrollLayout->addWidget(effectParamPanels);

     paramWidget->setLayout(scrollLayout);
     paramWidget->setStyleSheet("QWidget#paramWidget {background: qlineargradient(x1: 1, y1: 0, x2: 0, y2: 0, stop: 0 #777777, stop: 1 #F9F9F9);}");
     scrollArea->setWidget(paramWidget);

     QGridLayout *centralLayout = new QGridLayout;
#if 0
     centralLayout->addWidget(systemSelectorLabel, 0, 0);
     centralLayout->addWidget(systemSelector, 0, 1,1,1);
     centralLayout->addWidget(new QLabel("Renderer:"), 1, 0);
     centralLayout->addWidget(rendererSelector, 1, 1,1,1);
     centralLayout->addWidget(sphParams,2,0,7,2);
     centralLayout->addWidget(ssEffectParams,9,0,8,2);
     centralLayout->addWidget(glWidget, 0, 2,16,6);
#else
     centralLayout->addWidget(scrollArea,0,0,1,1,Qt::AlignLeft);
     centralLayout->addWidget(glWidget, 0, 1,1,6);
#endif


     centralWidget->setLayout(centralLayout);
     this->setStyleSheet("QMainWindow {background: qlineargradient(x1: 1, y1: 0, x2: 0, y2: 0, stop: 0 #777777, stop: 0.65 #777777, stop: 1 #F9F9F9);}");

     setWindowTitle(tr("RTPS Library Test"));
	 glWidget->makeCurrent();
     resize(1250, 700);
     showMaximized();
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
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"../test",tr("Wavefront (*.obj)"));
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
    systemSelector->blockSignals(true);
    systemSelector->clear();
    for(int i = 0;i<sysNames.size(); i++)
    {
        systemSelector->addItem(QString(sysNames[i].c_str()));
    }
    systemSelector->blockSignals(false);
}

void MainWindow::setRenderer(const QString& renderer)
{
    const QString& system = systemSelector->currentText();
    emit rendererChanged(system,renderer);
}

void MainWindow::setSystem(const QString& system)
{
    emit getSystemSettings(system);
}

void MainWindow::addRigidBody(const QString& mesh, float4 pos, float4 vel, float mass)
{
    emit addRigidBody(systemSelector->currentText(),mesh,pos,vel,mass);
}

void MainWindow::initRendererPanel(const QString& renderer, RTPSSettings* settings)
{
    effectParamPanels->setCurrentIndex(effectParamPanelID[renderer]);
    RTPSParameterGroup* panel = reinterpret_cast<RTPSParameterGroup*>(effectParamPanels->currentWidget());
    panel->setValues(settings);
}

void MainWindow::initSystemPanel(const QString& systemType, RTPSSettings* settings)
{
    systemParamPanels->setCurrentIndex(systemParamPanelID[systemType]);
    RTPSParameterGroup* panel = reinterpret_cast<RTPSParameterGroup*>(systemParamPanels->currentWidget());
    panel->setValues(settings);
}

void MainWindow::valueChanged(const QString& parameterName, const QString& value)
{
    const QString& system = systemSelector->currentText();
    emit parameterValueChanged(system,parameterName,value);
}
}
