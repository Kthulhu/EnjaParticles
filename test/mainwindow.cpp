#include <QtGui>
 #include <QtOpenGL>
#include <QComboBox>
 #include <QFileDialog>

 #include "glwidget.h"
 #include "mainwindow.h"
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

     connect(glWidget,SIGNAL(systemMapChanged(const std::vector<std::string>&)),this,SLOT(setSystemNames(const std::vector<std::string>&)));

     createActions();
     createMenus();

     QGridLayout *centralLayout = new QGridLayout;
     //centralLayout->addWidget(glWidgetArea, 0, 0);
     centralLayout->addWidget(systemSelector, 0, 0, 0, 1);
     centralLayout->addWidget(glWidget, 0, 1,0,4);
     
     centralWidget->setLayout(centralLayout);

     setWindowTitle(tr("RTPS Library Test"));
     resize(800, 600);
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
for(int i = 0;i<sysNames.size(); i++)
{
    systemSelector->addItem(QString(sysNames[i].c_str()));
}
}
 QSlider *MainWindow::createSlider(const char *changedSignal,
                                   const char *setterSlot)
 {
     QSlider *slider = new QSlider(Qt::Horizontal);
     slider->setRange(0, 360 * 16);
     slider->setSingleStep(16);
     slider->setPageStep(15 * 16);
     slider->setTickInterval(15 * 16);
     slider->setTickPosition(QSlider::TicksRight);
     connect(slider, SIGNAL(valueChanged(int)), glWidget, setterSlot);
     connect(glWidget, changedSignal, slider, SLOT(setValue(int)));
     return slider;
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
