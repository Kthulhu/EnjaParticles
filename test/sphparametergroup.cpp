 #include <QtGui>
 #include "floatslider.h"
 #include "sphparametergroup.h"
#include <iostream>

 SPHParameterGroup::SPHParameterGroup(Qt::Orientation orientation,
                            const QString &title,
                            QWidget *parent)
     : QGroupBox(title, parent)
 {
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
     viscositySlider->setTickInterval(1000);
     viscositySlider->setSingleStep(1000);
     viscositySlider->setRange(1,100000);
     viscositySlider->setValue(10000);
     viscositySlider->setScale(0.0001);

     viscosityLineEdit = new QLineEdit("0.001",this);
     viscosityLineEdit->setObjectName("viscosity");
     viscosityLineEdit->setMaxLength(5);
     viscosityLineEdit->setFixedWidth(50);

     gravityX = new QLineEdit("0.0",this);
     gravityX->setObjectName("gravityx");
     gravityX->setMaxLength(5);
     gravityX->setFixedWidth(50);
     gravityY = new QLineEdit("0.0",this);
     gravityY->setObjectName("gravityy");
     gravityY->setMaxLength(5);
     gravityY->setFixedWidth(50);
     gravityZ = new QLineEdit("-9.8",this);
     gravityZ->setObjectName("gravityz");
     gravityZ->setMaxLength(5);
     gravityZ->setFixedWidth(50);

     connect(xSPHSlider,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(xSPHSlider,SIGNAL(valueChanged(float)),this,SLOT(setXSPHValue(float)));
     connect(xSPHLineEdit,SIGNAL(textChanged(const QString&)),this,SLOT(setXSPHValue(const QString&)));
     connect(gasConstantSlider,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(gasConstantSlider,SIGNAL(valueChanged(float)),this,SLOT(setGasConstantValue(float)));
     connect(gasConstantLineEdit,SIGNAL(textChanged(const QString&)),this,SLOT(setGasConstantValue(const QString&)));
     connect(viscositySlider,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(viscositySlider,SIGNAL(valueChanged(float)),this,SLOT(setViscosityValue(float)));
     connect(viscosityLineEdit,SIGNAL(textChanged(const QString&)),this,SLOT(setViscosityValue(const QString&)));
     connect(gravityX,SIGNAL(textChanged(const QString&)),this,SLOT(triggerValue(const QString&)));
     connect(gravityY,SIGNAL(textChanged(const QString&)),this,SLOT(triggerValue(const QString&)));
     connect(gravityZ,SIGNAL(textChanged(const QString&)),this,SLOT(triggerValue(const QString&)));


     QBoxLayout::Direction direction;

     if (orientation == Qt::Horizontal)
         direction = QBoxLayout::TopToBottom;
     else
         direction = QBoxLayout::LeftToRight;

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
     slidersLayout->addWidget(createVectorInput("Gravity",gravityX,gravityY,gravityZ),3,0,1,3);
     setLayout(slidersLayout);

 }
QGroupBox* SPHParameterGroup::createVectorInput(const QString& title, QLineEdit* x,QLineEdit* y,QLineEdit* z)
{
     QGroupBox* box = new QGroupBox(title);
     QBoxLayout* boxLayout = new QBoxLayout(QBoxLayout::LeftToRight);
     boxLayout->addWidget(new QLabel("X:"));
     boxLayout->addWidget(x);
     boxLayout->addWidget(new QLabel("Y:"));
     boxLayout->addWidget(y);
     boxLayout->addWidget(new QLabel("Z:"));
     boxLayout->addWidget(z);
     box->setLayout(boxLayout);
     return box;
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

 void SPHParameterGroup::triggerValue(int value)
 {
     emit valueChanged(this->sender()->objectName(),QString::number(value));
 }
 void SPHParameterGroup::triggerValue(float value)
 {
     emit valueChanged(this->sender()->objectName(),QString::number(value));
 }
 void SPHParameterGroup::triggerValue(const QString& value)
 {
     emit valueChanged(this->sender()->objectName(),value);
 }
