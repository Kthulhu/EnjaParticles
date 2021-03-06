#include <QtGui>
#include <QLineEdit>
#include <QBoxLayout>
#include <QGridLayout>
#include "floatslider.h"
#include "sphparametergroup.h"

#include <iostream>
namespace rtps
{
 SPHParameterGroup::SPHParameterGroup(Qt::Orientation orientation,
                            const QString &title,
                            QWidget *parent)
     : RTPSParameterGroup(title, parent)
 {
     //integrator = new QComboBox(this);
     //integrator->setObjectName("integrator");
     //integrator->addItem("Euler");
     //integrator->addItem("Leapfrog");

     subIntervals = new QSlider(orientation,this);
     subIntervals->setObjectName("sub_intervals");
     subIntervals->setTickPosition(QSlider::TicksBelow);
     subIntervals->setTickInterval(10);
     subIntervals->setSingleStep(5);
     subIntervals->setRange(1,50);
     subIntervals->setValue(1);

     timeStep = new FloatSlider(orientation,this);
     timeStep->setObjectName("time_step");
     timeStep->setTickPosition(QSlider::TicksBelow);
     timeStep->setTickInterval(1);
     timeStep->setSingleStep(1);
     timeStep->setRange(50,150);
     timeStep->setValue(100);
     timeStep->setScale(0.00003);

     xSPHSlider = new FloatSlider(orientation,this);
     xSPHSlider->setObjectName("xsph_factor");
     xSPHSlider->setTickPosition(QSlider::TicksBelow);
     xSPHSlider->setTickInterval(10);
     xSPHSlider->setSingleStep(1);
     xSPHSlider->setRange(1,100);
     xSPHSlider->setValue(15);
     xSPHSlider->setScale(0.01);

     xSPHLineEdit = new QLineEdit("0.15",this);
     xSPHLineEdit->setObjectName("xsph_factor");
     xSPHLineEdit->setMaxLength(5);
     xSPHLineEdit->setFixedWidth(50);

     gasConstantSlider = new FloatSlider(orientation,this);
     gasConstantSlider->setObjectName("gas_constant");
     gasConstantSlider->setTickPosition(QSlider::TicksBelow);
     gasConstantSlider->setTickInterval(10);
     gasConstantSlider->setSingleStep(1);
     gasConstantSlider->setRange(1,100);
     gasConstantSlider->setValue(20);
     gasConstantSlider->setScale(0.05);

     gasConstantLineEdit = new QLineEdit("1.0",this);
     gasConstantLineEdit->setObjectName("gas_constant");
     gasConstantLineEdit->setMaxLength(5);
     gasConstantLineEdit->setFixedWidth(50);

     viscositySlider = new FloatSlider(orientation,this);
     viscositySlider->setObjectName("viscosity");
     viscositySlider->setTickPosition(QSlider::TicksBelow);
     viscositySlider->setTickInterval(100);
     viscositySlider->setSingleStep(10);
     viscositySlider->setRange(1,10000);
     viscositySlider->setValue(10);
     viscositySlider->setScale(0.00001);

     viscosityLineEdit = new QLineEdit("0.001",this);
     viscosityLineEdit->setObjectName("viscosity");
     viscosityLineEdit->setMaxLength(5);
     viscosityLineEdit->setFixedWidth(50);

     gravityX = new QLineEdit("0.0",this);
     gravityX->setObjectName("gravity_x");
     gravityX->setMaxLength(5);
     gravityX->setFixedWidth(50);
     gravityY = new QLineEdit("-9.8",this);
     gravityY->setObjectName("gravity_y");
     gravityY->setMaxLength(5);
     gravityY->setFixedWidth(50);
     gravityZ = new QLineEdit("0.0",this);
     gravityZ->setObjectName("gravity_z");
     gravityZ->setMaxLength(5);
     gravityZ->setFixedWidth(50);

     connect(xSPHSlider,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(xSPHSlider,SIGNAL(valueChanged(float)),this,SLOT(setXSPHValue(float)));
     connect(xSPHLineEdit,SIGNAL(textChanged(const QString&)),this,SLOT(setXSPHValue(const QString&)));
     connect(xSPHLineEdit,SIGNAL(textChanged(const QString&)),this,SLOT(triggerValue(const QString&)));
     connect(gasConstantSlider,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(gasConstantSlider,SIGNAL(valueChanged(float)),this,SLOT(setGasConstantValue(float)));
     connect(gasConstantLineEdit,SIGNAL(textChanged(const QString&)),this,SLOT(setGasConstantValue(const QString&)));
     connect(gasConstantLineEdit,SIGNAL(textChanged(const QString&)),this,SLOT(triggerValue(const QString&)));
     connect(viscositySlider,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(viscositySlider,SIGNAL(valueChanged(float)),this,SLOT(setViscosityValue(float)));
     connect(viscosityLineEdit,SIGNAL(textChanged(const QString&)),this,SLOT(setViscosityValue(const QString&)));
     connect(viscosityLineEdit,SIGNAL(textChanged(const QString&)),this,SLOT(triggerValue(const QString&)));
     connect(timeStep,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(subIntervals,SIGNAL(valueChanged(int)),this,SLOT(triggerValue(int)));
     connect(gravityX,SIGNAL(textChanged(const QString&)),this,SLOT(triggerVectorValue(const QString&)));
     connect(gravityY,SIGNAL(textChanged(const QString&)),this,SLOT(triggerVectorValue(const QString&)));
     connect(gravityZ,SIGNAL(textChanged(const QString&)),this,SLOT(triggerVectorValue(const QString&)));

     QGridLayout *slidersLayout = new QGridLayout();
     slidersLayout->addWidget(new QLabel("XSPH:"),0,0);
     slidersLayout->addWidget(xSPHLineEdit,0,1);
     slidersLayout->addWidget(xSPHSlider,0,2);
     slidersLayout->addWidget(new QLabel("Gas Constant:"),1,0);
     slidersLayout->addWidget(gasConstantLineEdit,1,1);
     slidersLayout->addWidget(gasConstantSlider,1,2);
     slidersLayout->addWidget(new QLabel("Viscosity:"),2,0);
     slidersLayout->addWidget(viscosityLineEdit,2,1);
     slidersLayout->addWidget(viscositySlider,2,2);
     slidersLayout->addWidget(new QLabel("Time Step:"),3,0);
     slidersLayout->addWidget(timeStep,3,1,1,2);
     slidersLayout->addWidget(new QLabel("Sub-intervals:"),4,0);
     slidersLayout->addWidget(subIntervals,4,1,1,2);
     slidersLayout->addWidget(createVectorInput("Gravity",gravityX,gravityY,gravityZ),5,0,1,3);
     setLayout(slidersLayout);

 }

void SPHParameterGroup::setXSPHValue(float value)
{
    xSPHLineEdit->blockSignals(true);
    xSPHLineEdit->setText(QString::number(value));
    xSPHLineEdit->blockSignals(false);
}

void SPHParameterGroup::setGasConstantValue(float value)
{
    gasConstantLineEdit->blockSignals(true);
    gasConstantLineEdit->setText(QString::number(value));
    gasConstantLineEdit->blockSignals(false);
}

void SPHParameterGroup::setViscosityValue(float value)
{
    viscosityLineEdit->blockSignals(true);
    viscosityLineEdit->setText(QString::number(value));
    viscosityLineEdit->blockSignals(false);
}

void SPHParameterGroup::setXSPHValue(const QString& value)
{
    xSPHSlider->blockSignals(true);
    xSPHSlider->setValue(value.toFloat());
    xSPHSlider->blockSignals(false);
}

void SPHParameterGroup::setGasConstantValue(const QString& value)
{
    gasConstantSlider->blockSignals(true);
    gasConstantSlider->setValue(value.toFloat());
    gasConstantSlider->blockSignals(false);
}

void SPHParameterGroup::setViscosityValue(const QString& value)
{
    viscositySlider->blockSignals(true);
    viscositySlider->setValue(value.toFloat());
    viscositySlider->blockSignals(false);
}

void SPHParameterGroup::setValues(RTPSSettings *settings)
{
    xSPHLineEdit->blockSignals(true);
    xSPHLineEdit->setText(QString(settings->GetSettingAs<std::string>("xsph_factor","0.15").c_str()));
    xSPHLineEdit->blockSignals(false);
    gasConstantLineEdit->blockSignals(true);
    gasConstantLineEdit->setText(QString(settings->GetSettingAs<std::string>("gas_constant","3.5").c_str()));
    gasConstantLineEdit->blockSignals(false);
    viscosityLineEdit->blockSignals(true);
    viscosityLineEdit->setText(QString(settings->GetSettingAs<std::string>("viscosity","0.001").c_str()));
    viscosityLineEdit->blockSignals(false);
    xSPHSlider->blockSignals(true);
    xSPHSlider->setValue(settings->GetSettingAs<float>("xsph_factor","0.15"));
    xSPHSlider->blockSignals(false);
    gasConstantSlider->blockSignals(true);
    gasConstantSlider->setValue(settings->GetSettingAs<float>("gas_constant","3.5"));
    gasConstantSlider->blockSignals(false);
    viscositySlider->blockSignals(true);
    viscositySlider->setValue(settings->GetSettingAs<float>("viscosity","0.001"));
    viscositySlider->blockSignals(false);
    timeStep->blockSignals(true);
    timeStep->setValue(settings->GetSettingAs<float>("time_step","0.003"));
    timeStep->blockSignals(false);
    subIntervals->blockSignals(true);
    subIntervals->setValue(settings->GetSettingAs<int>("sub_intervals","1"));
    subIntervals->blockSignals(false);
    //QString gravity = settings->GetSettingAs<std::string>("gravity","0.0 -9.8 0.0 0.0").c_str();
    float4 gravity = settings->GetSettingAs<float4>("gravity","0.0 -9.8 0.0 0.0");
    //std::cout<<"Here!"<<endl;
    //std::cout<<"gravity "<<gravity.toAscii().data()<<"!"<<endl;
    //std::cout<<"gravity "<<gravity.section(' ',0,0).toAscii().data()<<","<<gravity.section(' ',1,1).toAscii().data()<<","<<gravity.section(' ',2,2).toAscii().data()<<std::endl;
    gravityX->blockSignals(true);
    gravityY->blockSignals(true);
    gravityZ->blockSignals(true);
    gravityX->setText(QString::number(gravity.x));
    gravityY->setText(QString::number(gravity.y));
    gravityZ->setText(QString::number(gravity.z));
    gravityX->blockSignals(false);
    gravityY->blockSignals(false);
    gravityZ->blockSignals(false);
}
}
