 #include <QtGui>
 #include "floatslider.h"
 #include "sphparametergroup.h"
#include <iostream>

 ParticleEffectParameterGroup::SPHParameterGroup(Qt::Orientation orientation,
                            const QString &title,
                            QWidget *parent)
     : QGroupBox(title, parent)
 {
     pointScale = new FloatSlider(orientation,this);
     pointScaleSlider->setObjectName("xsph_factor");
     pointScaleSlider->setTickPosition(QSlider::TicksBelow);
     pointScaleSlider->setTickInterval(10);
     pointScaleSlider->setSingleStep(1);
     pointScaleSlider->setRange(1,100);
     pointScaleSlider->setValue(15);
     pointScaleSlider->setScale(0.01);

     gasConstantSlider = new FloatSlider(orientation,this);
     gasConstantSlider->setObjectName("gas_constant");
     gasConstantSlider->setTickPosition(QSlider::TicksBelow);
     gasConstantSlider->setTickInterval(10);
     gasConstantSlider->setSingleStep(1);
     gasConstantSlider->setRange(1,100);
     gasConstantSlider->setValue(20);
     gasConstantSlider->setScale(0.05);

     viscositySlider = new FloatSlider(orientation,this);
     viscositySlider->setObjectName("viscosity");
     viscositySlider->setTickPosition(QSlider::TicksBelow);
     viscositySlider->setTickInterval(100);
     viscositySlider->setSingleStep(1);
     viscositySlider->setRange(1,100000);
     viscositySlider->setValue(10000);
     viscositySlider->setScale(0.00001);


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

     connect(pointScaleSlider,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(gasConstantSlider,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(viscositySlider,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
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
     slidersLayout->addWidget(pointScaleSlider,0,1);
     slidersLayout->addWidget(new QLabel("Gas Constant:"),1,0);
     slidersLayout->addWidget(gasConstantSlider,1,1);
     slidersLayout->addWidget(new QLabel("Viscosity:"),2,0);
     slidersLayout->addWidget(viscositySlider,2,1);
     slidersLayout->addWidget(createVectorInput("Gravity",gravityX,gravityY,gravityZ),3
     setLayout(slidersLayout);

 }
QGroupBox* ParticleEffectParameterGroup::createVectorInput(const QString& title, QLineEdit* x,QLineEdit* y,QLineEdit* z)
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
void ParticleEffectParameterGroup::setValue(int value)
{
    const QString& parameter = this->sender()->objectName();
}
void ParticleEffectParameterGroup::setValue(float value)
{
    const QString& parameter = this->sender()->objectName();
}
void ParticleEffectParameterGroup::setValue(const QString& value)
{
    const QString& parameter = this->sender()->objectName();

}
 void ParticleEffectParameterGroup::triggerValue(int value)
 {
     emit valueChanged(this->sender()->objectName(),QString::number(value));
 }
 void ParticleEffectParameterGroup::triggerValue(float value)
 {
     std::cout<<"name = "<<(const char*)this->sender()->objectName().toAscii().data() <<" value = "<<value<<std::endl;
     emit valueChanged(this->sender()->objectName(),QString::number(value));
 }
 void ParticleEffectParameterGroup::triggerValue(const QString& value)
 {
     emit valueChanged(this->sender()->objectName(),value);
 }
