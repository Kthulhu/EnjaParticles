#include <QtGui>
#include <QLineEdit>
#include <QBoxLayout>
#include <QGridLayout>
#include <QComboBox>
#include "floatslider.h"
#include "rigidbodyparametergroup.h"

#include <iostream>
namespace rtps
{
 RigidbodyParameterGroup::RigidbodyParameterGroup(Qt::Orientation orientation,
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

     velocityLimit = new FloatSlider(orientation,this);
     velocityLimit->setObjectName("velocity_limit");
     velocityLimit->setTickPosition(QSlider::TicksBelow);
     velocityLimit->setTickInterval(5);
     velocityLimit->setSingleStep(1);
     velocityLimit->setRange(50,150);
     velocityLimit->setValue(100);
     velocityLimit->setScale(6.0);

     frictionDynamic = new FloatSlider(orientation,this);
     frictionDynamic->setObjectName("friction_dynamic");
     frictionDynamic->setTickPosition(QSlider::TicksBelow);
     frictionDynamic->setTickInterval(5);
     frictionDynamic->setSingleStep(1);
     frictionDynamic->setRange(1,1000);
     frictionDynamic->setValue(100);
     frictionDynamic->setScale(0.001);

     frictionStatic = new FloatSlider(orientation,this);
     frictionStatic->setObjectName("friction_static");
     frictionStatic->setTickPosition(QSlider::TicksBelow);
     frictionStatic->setTickInterval(5);
     frictionStatic->setSingleStep(1);
     frictionStatic->setRange(1,1000);
     frictionStatic->setValue(100);
     frictionStatic->setScale(0.001);

     frictionStaticThreshold = new FloatSlider(orientation,this);
     frictionStaticThreshold->setObjectName("friction_static_threshold");
     frictionStaticThreshold->setTickPosition(QSlider::TicksBelow);
     frictionStaticThreshold->setTickInterval(1);
     frictionStaticThreshold->setSingleStep(1);
     frictionStaticThreshold->setRange(1,50);
     frictionStaticThreshold->setValue(10);
     frictionStaticThreshold->setScale(0.00001);

     penetrationFactor = new FloatSlider(orientation,this);
     penetrationFactor->setObjectName("penetration_factor");
     penetrationFactor->setTickPosition(QSlider::TicksBelow);
     penetrationFactor->setTickInterval(1);
     penetrationFactor->setSingleStep(1);
     penetrationFactor->setRange(100,300);
     penetrationFactor->setValue(200);
     penetrationFactor->setScale(0.0001);

     restitution = new FloatSlider(orientation,this);
     restitution->setObjectName("restitution");
     restitution->setTickPosition(QSlider::TicksBelow);
     restitution->setTickInterval(5);
     restitution->setSingleStep(5);
     restitution->setRange(1,1000);
     restitution->setValue(600);
     restitution->setScale(0.001);

     timeStep = new FloatSlider(orientation,this);
     timeStep->setObjectName("time_step");
     timeStep->setTickPosition(QSlider::TicksBelow);
     timeStep->setTickInterval(1);
     timeStep->setSingleStep(1);
     timeStep->setRange(50,150);
     timeStep->setValue(100);
     timeStep->setScale(0.00003);

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

     meshSelection = new QComboBox(this);

     posX = new QLineEdit("5.0",this);
     posX->setObjectName("pos_x");
     posX->setMaxLength(5);
     posX->setFixedWidth(50);
     posY = new QLineEdit("10.0",this);
     posY->setObjectName("pos_y");
     posY->setMaxLength(5);
     posY->setFixedWidth(50);
     posZ = new QLineEdit("5.0",this);
     posZ->setObjectName("pos_z");
     posZ->setMaxLength(5);
     posZ->setFixedWidth(50);

     velX = new QLineEdit("0.0",this);
     velX->setObjectName("vel_x");
     velX->setMaxLength(5);
     velX->setFixedWidth(50);
     velY = new QLineEdit("-0.01",this);
     velY->setObjectName("vel_y");
     velY->setMaxLength(5);
     velY->setFixedWidth(50);
     velZ = new QLineEdit("0.0",this);
     velZ->setObjectName("vel_z");
     velZ->setMaxLength(5);
     velZ->setFixedWidth(50);

     mass = new QLineEdit("0.1",this);

     addRigidBodyButton = new QPushButton("&Add Rigid Body", this);

     connect(timeStep,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(restitution,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(penetrationFactor,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(frictionStaticThreshold,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(frictionStatic,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(frictionDynamic,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(velocityLimit,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(subIntervals,SIGNAL(valueChanged(int)),this,SLOT(triggerValue(int)));
     connect(addRigidBodyButton,SIGNAL(clicked()),this,SLOT(addRigidBody()));
     connect(gravityX,SIGNAL(textChanged(const QString&)),this,SLOT(triggerVectorValue(const QString&)));
     connect(gravityY,SIGNAL(textChanged(const QString&)),this,SLOT(triggerVectorValue(const QString&)));
     connect(gravityZ,SIGNAL(textChanged(const QString&)),this,SLOT(triggerVectorValue(const QString&)));

     QGridLayout *slidersLayout = new QGridLayout();
     slidersLayout->addWidget(new QLabel("Velcoity Limit:"),0,0);
     slidersLayout->addWidget(velocityLimit,0,1,1,2);
     slidersLayout->addWidget(new QLabel("Restitution:"),1,0);
     slidersLayout->addWidget(restitution,1,1,1,2);
     slidersLayout->addWidget(new QLabel("Penetration Factor:"),2,0);
     slidersLayout->addWidget(penetrationFactor,2,1,1,2);
     slidersLayout->addWidget(new QLabel("Friction Dynamic:"),3,0);
     slidersLayout->addWidget(frictionDynamic,3,1,1,2);
     slidersLayout->addWidget(new QLabel("Friction Static:"),4,0);
     slidersLayout->addWidget(frictionStatic,4,1,1,2);
     slidersLayout->addWidget(new QLabel("Friction Static Threshold:"),5,0);
     slidersLayout->addWidget(frictionStaticThreshold,5,1,1,2);
     slidersLayout->addWidget(new QLabel("Time Step:"),6,0);
     slidersLayout->addWidget(timeStep,6,1,1,2);
     slidersLayout->addWidget(new QLabel("Sub-intervals:"),7,0);
     slidersLayout->addWidget(subIntervals,7,1,1,2);
     slidersLayout->addWidget(createVectorInput("Gravity",gravityX,gravityY,gravityZ),8,0,1,3);
     slidersLayout->addWidget(addRigidBodyButton,9,0,1,3);
     slidersLayout->addWidget(meshSelection,10,0,1,3);
     slidersLayout->addWidget(createVectorInput("Initial Position",posX,posY,posZ),11,0,1,3);
     slidersLayout->addWidget(createVectorInput("Velocity Position",velX,velY,velZ),12,0,1,3);
     slidersLayout->addWidget(new QLabel("Mass:"),13,0);
     slidersLayout->addWidget(mass,13,1,1,3);
     setLayout(slidersLayout);

 }

void RigidbodyParameterGroup::addRigidBody()
{
    float4 pos;
    float4 vel;
    float fMass;
    pos.x = posX->displayText().toFloat();
    pos.y = posY->displayText().toFloat();
    pos.z = posZ->displayText().toFloat();
    pos.w = 0.0f;
    vel.x = velX->displayText().toFloat();
    vel.y = velY->displayText().toFloat();
    vel.z = velZ->displayText().toFloat();
    vel.w = 0.0f;
    fMass = mass->displayText().toFloat();
    emit addRigidBody(meshSelection->currentText(),pos,vel,fMass);
}

void RigidbodyParameterGroup::meshListUpdated(const std::vector<QString>& meshes)
{
    meshSelection->blockSignals(true);
    meshSelection->clear();
    for(std::vector<QString>::const_iterator i = meshes.begin(); i!=meshes.end(); i++)
    {
        meshSelection->addItem(*i);
    }
    meshSelection->blockSignals(false);
}

void RigidbodyParameterGroup::setValues(RTPSSettings *settings)
{
    timeStep->blockSignals(true);
    restitution->blockSignals(true);
    penetrationFactor->blockSignals(true);
    frictionStaticThreshold->blockSignals(true);
    frictionStatic->blockSignals(true);
    frictionDynamic->blockSignals(true);
    velocityLimit->blockSignals(true);
    subIntervals->blockSignals(true);
    timeStep->setValue(settings->GetSettingAs<float>("time_step","0.003"));
    restitution->setValue(settings->GetSettingAs<float>("restitution","0.605"));
    penetrationFactor->setValue(settings->GetSettingAs<float>("penetration_factor","0.01"));
    frictionStaticThreshold->setValue(settings->GetSettingAs<float>("friction_static_threshold","0.0001"));
    frictionStatic->setValue(settings->GetSettingAs<float>("friction_static","0.01"));
    frictionDynamic->setValue(settings->GetSettingAs<float>("friction_dynamic","0.02"));
    velocityLimit->setValue(settings->GetSettingAs<float>("velocity_limit","600.0"));
    subIntervals->setValue(settings->GetSettingAs<float>("sub_intervals","1"));
    timeStep->blockSignals(false);
    restitution->blockSignals(false);
    penetrationFactor->blockSignals(false);
    frictionStaticThreshold->blockSignals(false);
    frictionStatic->blockSignals(false);
    frictionDynamic->blockSignals(false);
    velocityLimit->blockSignals(false);
    subIntervals->blockSignals(false);
    float4 gravity = settings->GetSettingAs<float4>("gravity","0.0 -9.8 0.0 0.0");
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
