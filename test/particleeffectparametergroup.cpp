#include <QtGui>
#include <QRadioButton>
#include <QButtonGroup>
#include <iostream>
#include "floatslider.h"
#include "particleeffectparametergroup.h"
namespace rtps
{
 ParticleEffectParameterGroup::ParticleEffectParameterGroup(Qt::Orientation orientation,
                            const QString &title,
                            QWidget *parent)
     : RTPSParameterGroup(title, parent)
 {
     pointScaleSlider = new FloatSlider(orientation,this);
     pointScaleSlider->setObjectName("point_radius");
     pointScaleSlider->setTickPosition(QSlider::TicksBelow);
     pointScaleSlider->setTickInterval(10);
     pointScaleSlider->setSingleStep(5);
     pointScaleSlider->setRange(1,500);
     pointScaleSlider->setValue(15);
     pointScaleSlider->setScale(0.01);

     blurRadius = new FloatSlider(orientation,this);
     blurRadius->setObjectName("blur_radius");
     blurRadius->setTickPosition(QSlider::TicksBelow);
     blurRadius->setTickInterval(10);
     blurRadius->setSingleStep(1);
     blurRadius->setRange(1,100);
     blurRadius->setValue(15);
     blurRadius->setScale(0.5);

     bilateralRange = new FloatSlider(orientation,this);
     bilateralRange->setObjectName("bilateral_range");
     bilateralRange->setTickPosition(QSlider::TicksBelow);
     bilateralRange->setTickInterval(10);
     bilateralRange->setSingleStep(1);
     bilateralRange->setRange(1,1000);
     bilateralRange->setValue(10);
     bilateralRange->setScale(0.0001);

     thicknessGamma = new FloatSlider(orientation,this);
     thicknessGamma->setObjectName("thickness_gamma");
     thicknessGamma->setTickPosition(QSlider::TicksBelow);
     thicknessGamma->setTickInterval(10);
     thicknessGamma->setSingleStep(1);
     thicknessGamma->setRange(1,100);
     thicknessGamma->setValue(10);
     thicknessGamma->setScale(0.01);

     curvatureFlowIterations = new QSlider(orientation,this);
     curvatureFlowIterations->setObjectName("curvature_flow_iterations");
     curvatureFlowIterations->setTickPosition(QSlider::TicksBelow);
     curvatureFlowIterations->setTickInterval(10);
     curvatureFlowIterations->setSingleStep(1);
     curvatureFlowIterations->setRange(1,100);
     curvatureFlowIterations->setValue(15);

     curvatureFlowDT = new FloatSlider(orientation,this);
     curvatureFlowDT->setObjectName("curvature_flow_dt");
     curvatureFlowDT->setTickPosition(QSlider::TicksBelow);
     curvatureFlowDT->setTickInterval(1000);
     curvatureFlowDT->setSingleStep(10);
     curvatureFlowDT->setRange(1,10000);
     curvatureFlowDT->setValue(100);
     curvatureFlowDT->setScale(0.000005);

     velocityScale = new FloatSlider(orientation,this);
     velocityScale->setObjectName("velocity_scale");
     velocityScale->setTickPosition(QSlider::TicksBelow);
     velocityScale->setTickInterval(10);
     velocityScale->setSingleStep(1);
     velocityScale->setRange(1,100);
     velocityScale->setValue(10);
     velocityScale->setScale(0.01);


     filterType = new QComboBox(this);
     filterType->setObjectName("filter_type");
     filterType->addItem("No Smoothing");
     filterType->addItem("Gaussian Blur");
     filterType->addItem("Seperable Gaussian Blur");
     filterType->addItem("Bilateral Gaussian Blur");
     filterType->addItem("Curvature Flow");
     thicknessCheck = new QCheckBox("Enable Thickness",this);
     thicknessCheck->setObjectName("thickness");
     velocityCheck = new QCheckBox("Enable Velocity",this);
     velocityCheck->setObjectName("render_velocity");
     //streamlinesCheck = new QCheckBox("Enable Streamlines",this);
     //streamlinesCheck->setObjectName("render_streamlines");
     renderButtonGroup = new QGroupBox("Render Buffer",this);
     renderNormal=new QRadioButton("Normal");//,renderButtonGroup);
     renderNormal->setObjectName("render_normal");
     renderDepth=new QRadioButton("Depth");//,renderButtonGroup);
     renderDepth->setObjectName("render_depth");
     renderDepthSmoothed=new QRadioButton("Smoothed Depth");//,renderButtonGroup);
     renderDepthSmoothed->setObjectName("render_depth_smoothed");
     renderThickness=new QRadioButton("Thickness");//,renderButtonGroup);
     renderThickness->setObjectName("render_thickness");
     renderComposite=new QRadioButton("Composite");//-,renderButtonGroup);
     renderComposite->setObjectName("render_composite");
     renderComposite->setChecked(true);
     //TODO: Make enum for buffer types to render...
     //renderButtonGroup->addButton(renderNormal,0);
     //renderButtonGroup->addButton(renderDepth,1);
     //renderButtonGroup->addButton(renderDepthSmoothed,2);
     //renderButtonGroup->addButton(renderThickness,3);
     //renderButtonGroup->addButton(renderComposite,4);

     connect(filterType,SIGNAL(currentIndexChanged(int)),this,SLOT(triggerValue(int)));
     connect(pointScaleSlider,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(blurRadius,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(bilateralRange,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(curvatureFlowIterations,SIGNAL(valueChanged(int)),this,SLOT(triggerValue(int)));
     connect(thicknessGamma,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(curvatureFlowDT,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(velocityScale,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(thicknessCheck,SIGNAL(stateChanged(int)),this,SLOT(triggerValue(int)));
     connect(velocityCheck,SIGNAL(stateChanged(int)),this,SLOT(triggerValue(int)));
     //connect(streamlinesCheck,SIGNAL(stateChanged(int)),this,SLOT(triggerValue(int)));
     connect(renderNormal,SIGNAL(toggled(bool)),this,SLOT(triggerValue(bool)));
     connect(renderDepth,SIGNAL(toggled(bool)),this,SLOT(triggerValue(bool)));
     connect(renderDepthSmoothed,SIGNAL(toggled(bool)),this,SLOT(triggerValue(bool)));
     connect(renderThickness,SIGNAL(toggled(bool)),this,SLOT(triggerValue(bool)));
     connect(renderComposite,SIGNAL(toggled(bool)),this,SLOT(triggerValue(bool)));

     QBoxLayout::Direction direction;

     if (orientation == Qt::Horizontal)
         direction = QBoxLayout::TopToBottom;
     else
         direction = QBoxLayout::LeftToRight;

     QGridLayout *slidersLayout = new QGridLayout();
     slidersLayout->addWidget(new QLabel("Filter Type:"),0,0);
     slidersLayout->addWidget(filterType,0,1);
     slidersLayout->addWidget(new QLabel("Point Scale:"),1,0);
     slidersLayout->addWidget(pointScaleSlider,1,1);
     slidersLayout->addWidget(new QLabel("Blur Radius:"),2,0);
     slidersLayout->addWidget(blurRadius,2,1);
     slidersLayout->addWidget(new QLabel("Bilateral Range:"),3,0);
     slidersLayout->addWidget(bilateralRange,3,1);
     slidersLayout->addWidget(new QLabel("Curvature Flow Iterations:"),4,0);
     slidersLayout->addWidget(curvatureFlowIterations,4,1);
     slidersLayout->addWidget(new QLabel("Curvature Flow DT:"),5,0);
     slidersLayout->addWidget(curvatureFlowDT,5,1);
     slidersLayout->addWidget(new QLabel("Thickness Gamma:"),6,0);
     slidersLayout->addWidget(thicknessGamma,6,1);
     slidersLayout->addWidget(new QLabel("Velocity Scale:"),7,0);
     slidersLayout->addWidget(velocityScale,7,1);
     //slidersLayout->addWidget(new QLabel("Curvature Flow Iterations:"),4,0);
     slidersLayout->addWidget(velocityCheck,8,0);
     slidersLayout->addWidget(thicknessCheck,8,1);
     //slidersLayout->addWidget(streamlinesCheck,9,0);
     //slidersLayout->addWidget(new QLabel("Render Buffer:"),6,0);
     QGridLayout *buttonGroupLayout = new QGridLayout();
     buttonGroupLayout->addWidget(renderNormal,0,0);
     buttonGroupLayout->addWidget(renderDepth,1,0);
     buttonGroupLayout->addWidget(renderDepthSmoothed,3,0);
     buttonGroupLayout->addWidget(renderThickness,0,1);
     buttonGroupLayout->addWidget(renderComposite,1,1);
     renderButtonGroup->setLayout(buttonGroupLayout);
     slidersLayout->addWidget(renderButtonGroup,10,0,1,2);

     setLayout(slidersLayout);


 }
 void ParticleEffectParameterGroup::setValues(RTPSSettings* settings)
 {
     //TODO: implement this function for particle effects panel.
 }

}
