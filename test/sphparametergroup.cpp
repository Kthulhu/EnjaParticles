 #include <QtGui>

 #include "sphparametergroup.h"

 SPHParameterGroup::SPHParameterGroup(Qt::Orientation orientation,
                            const QString &title,
                            QWidget *parent)
     : QGroupBox(title, parent)
 {
     xSPHSlider = new QSlider(orientation,this);
     xSPHSlider->setObjectName("xsph");
     xSPHSlider->setTickPosition(QSlider::TicksBothSides);
     xSPHSlider->setTickInterval(10);
     xSPHSlider->setSingleStep(1);
     xSPHSlider->setRange(0,100);


     gravityX = new QLineEdit("0.0",this);
     gravityX->setMaxLength(5);
     gravityX->setFixedWidth(50);
     gravityY = new QLineEdit("0.0",this);
     gravityY->setMaxLength(5);
     gravityY->setFixedWidth(50);
     gravityZ = new QLineEdit("-9.8",this);
     gravityZ->setMaxLength(5);
     gravityZ->setFixedWidth(50);

     connect(xSPHSlider,SIGNAL(valueChanged(int)),this,SLOT(triggerValue(int)));
     connect(gravityX,SIGNAL(valueChanged(const QString&)),this,SLOT(triggerValue(const QString&)));
     connect(gravityY,SIGNAL(valueChanged(const QString&)),this,SLOT(triggerValue(const QString&)));
     connect(gravityZ,SIGNAL(valueChanged(const QString&)),this,SLOT(triggerValue(const QString&)));


     QBoxLayout::Direction direction;

     if (orientation == Qt::Horizontal)
         direction = QBoxLayout::TopToBottom;
     else
         direction = QBoxLayout::LeftToRight;

     QGridLayout *slidersLayout = new QGridLayout();
     slidersLayout->addWidget(new QLabel("XSPH:"),0,0);
     slidersLayout->addWidget(xSPHSlider,0,1);
     slidersLayout->addWidget(createVectorInput("Gravity",gravityX,gravityY,gravityZ),1,0,1,2);
     slidersLayout->addWidget(new QLabel(""),3,0);
     slidersLayout->addWidget(new QLabel(""),4,0);
     slidersLayout->addWidget(new QLabel(""),5,0);
     slidersLayout->addWidget(new QLabel(""),6,0);
     slidersLayout->addWidget(new QLabel(""),7,0);
     slidersLayout->addWidget(new QLabel(""),8,0);
     slidersLayout->addWidget(new QLabel(""),9,0);
     slidersLayout->addWidget(new QLabel(""),10,0);
     slidersLayout->addWidget(new QLabel(""),11,0);
     slidersLayout->addWidget(new QLabel(""),12,0);
     slidersLayout->addWidget(new QLabel(""),13,0);
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
void SPHParameterGroup::setValue(int value)
{
    const QString& parameter = this->sender()->objectName();

}
void SPHParameterGroup::setValue(const QString& value)
{
    const QString& parameter = this->sender()->objectName();

}
 void SPHParameterGroup::triggerValue(int value)
 {
     emit valueChanged(value);
 }
 void SPHParameterGroup::triggerValue(const QString& value)
 {
     emit valueChanged(value);
 }
