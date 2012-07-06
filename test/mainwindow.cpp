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

     /*QSlider* slider1 = createSlider("test1");
     QSlider* slider2 = createSlider("test2");
     QLabel* slider1Label = new QLabel("Slider1:",this);
     QLabel* slider2Label = new QLabel("Slider2:",this);*/
     sphParams = new SPHParameterGroup(Qt::Horizontal, "SPH Parameters",this);
     connect(sphParams, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));

     connect(glWidget,SIGNAL(systemMapChanged(const std::vector<std::string>&)),this,SLOT(setSystemNames(const std::vector<std::string>&)));

     createActions();
     createMenus();

     QGridLayout *centralLayout = new QGridLayout;
     //centralLayout->addWidget(glWidgetArea, 0, 0);
     centralLayout->addWidget(systemSelectorLabel, 0, 0);
     centralLayout->addWidget(systemSelector, 0, 1,1,1);
     /*centralLayout->addWidget(slider1Label, 1, 0);
     centralLayout->addWidget(slider1, 1, 1 );
     centralLayout->addWidget(slider2Label, 2, 0);
     centralLayout->addWidget(slider2, 2, 1);*/
     centralLayout->addWidget(sphParams,1,0,14,2);
     centralLayout->addWidget(glWidget, 0, 2,15,6);

     centralWidget->setLayout(centralLayout);

     setWindowTitle(tr("RTPS Library Test"));
     resize(1250, 600);
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
 QSlider *MainWindow::createSlider(const char *name)
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
 }

void MainWindow::sliderChanged(int value)
{
    QString system = systemSelector->currentText();
    const QString& parameter = this->sender()->objectName();
    dout<<"sys = "<<(const char*)system.toAscii().data()<<" parameter = "<<(const char*)parameter.toAscii().data()<<" value = "<<value<<endl;
}

 QSize MainWindow::getSize()
 {
     bool ok;
     QString text = QInputDialog::getText(this, tr("Grabber"),
                                          tr("Enter pixmap size:"),
                                          QLineEdit::Normal,
                                          tr("%1 x %2").arg(glWidget->width())
                                                       .arg(glWidget->height()),
                                          &ok);
     if (!ok)
         return QSize();

     QRegExp regExp(tr("([0-9]+) *x *([0-9]+)"));
     if (regExp.exactMatch(text)) {
         int width = regExp.cap(1).toInt();
         int height = regExp.cap(2).toInt();
         if (width > 0 && width < 2048 && height > 0 && height < 2048)
             return QSize(width, height);
     }

     return glWidget->size();
 }
}
