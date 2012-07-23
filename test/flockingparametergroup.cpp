#include <QtGui>
#include <QLineEdit>
#include <QBoxLayout>
#include <QGridLayout>
#include "floatslider.h"
#include "flockingparametergroup.h"

#include <iostream>
namespace rtps
{
 FlockingParameterGroup::FlockingParameterGroup(Qt::Orientation orientation,
                            const QString &title,
                            QWidget *parent)
     : RTPSParameterGroup(title, parent)
 {
     integrator = new QComboBox(this);
     integrator->setObjectName("integrator");
     integrator->addItem("Euler");
     integrator->addItem("Leapfrog");

     minSeparationDistance = new FloatSlider(orientation,this);
     minSeparationDistance->setObjectName("min_separation_distance");
     minSeparationDistance->setTickPosition(QSlider::TicksBelow);
     minSeparationDistance->setTickInterval(5);
     minSeparationDistance->setSingleStep(1);
     minSeparationDistance->setRange(50,150);
     minSeparationDistance->setValue(100);
     minSeparationDistance->setScale(6.0);

     searchingRadius = new FloatSlider(orientation,this);
     searchingRadius->setObjectName("searching_radius");
     searchingRadius->setTickPosition(QSlider::TicksBelow);
     searchingRadius->setTickInterval(5);
     searchingRadius->setSingleStep(1);
     searchingRadius->setRange(50,150);
     searchingRadius->setValue(100);
     searchingRadius->setScale(6.0);

     maxSpeed = new FloatSlider(orientation,this);
     maxSpeed->setObjectName("max_speed");
     maxSpeed->setTickPosition(QSlider::TicksBelow);
     maxSpeed->setTickInterval(5);
     maxSpeed->setSingleStep(1);
     maxSpeed->setRange(50,150);
     maxSpeed->setValue(100);
     maxSpeed->setScale(6.0);

     angularVelocity = new FloatSlider(orientation,this);
     angularVelocity->setObjectName("angular_velocity");
     angularVelocity->setTickPosition(QSlider::TicksBelow);
     angularVelocity->setTickInterval(1);
     angularVelocity->setSingleStep(1);
     angularVelocity->setRange(1,50);
     angularVelocity->setValue(10);
     angularVelocity->setScale(0.00001);

     seperationWeight = new FloatSlider(orientation,this);
     seperationWeight->setObjectName("seperation_weight");
     seperationWeight->setTickPosition(QSlider::TicksBelow);
     seperationWeight->setTickInterval(5);
     seperationWeight->setSingleStep(5);
     seperationWeight->setRange(1,1000);
     seperationWeight->setValue(600);
     seperationWeight->setScale(0.001);

     alignmentWeight = new FloatSlider(orientation,this);
     alignmentWeight->setObjectName("alignment_weight");
     alignmentWeight->setTickPosition(QSlider::TicksBelow);
     alignmentWeight->setTickInterval(5);
     alignmentWeight->setSingleStep(5);
     alignmentWeight->setRange(1,1000);
     alignmentWeight->setValue(20);
     alignmentWeight->setScale(0.001);

     cohesionWeight = new FloatSlider(orientation,this);
     cohesionWeight->setObjectName("cohesion_weight");
     cohesionWeight->setTickPosition(QSlider::TicksBelow);
     cohesionWeight->setTickInterval(5);
     cohesionWeight->setSingleStep(5);
     cohesionWeight->setRange(1,1000);
     cohesionWeight->setValue(20);
     cohesionWeight->setScale(0.001);

     goalWeight = new FloatSlider(orientation,this);
     goalWeight->setObjectName("goal_weight");
     goalWeight->setTickPosition(QSlider::TicksBelow);
     goalWeight->setTickInterval(5);
     goalWeight->setSingleStep(5);
     goalWeight->setRange(1,1000);
     goalWeight->setValue(20);
     goalWeight->setScale(0.001);

     avoidWeight = new FloatSlider(orientation,this);
     avoidWeight->setObjectName("avoid_weight");
     avoidWeight->setTickPosition(QSlider::TicksBelow);
     avoidWeight->setTickInterval(5);
     avoidWeight->setSingleStep(5);
     avoidWeight->setRange(1,1000);
     avoidWeight->setValue(20);
     avoidWeight->setScale(0.001);

     wanderWeight = new FloatSlider(orientation,this);
     wanderWeight->setObjectName("wander_weight");
     wanderWeight->setTickPosition(QSlider::TicksBelow);
     wanderWeight->setTickInterval(5);
     wanderWeight->setSingleStep(5);
     wanderWeight->setRange(1,1000);
     wanderWeight->setValue(20);
     wanderWeight->setScale(0.001);

     leaderFollowingWeight = new FloatSlider(orientation,this);
     leaderFollowingWeight->setObjectName("leader_following_weight");
     leaderFollowingWeight->setTickPosition(QSlider::TicksBelow);
     leaderFollowingWeight->setTickInterval(5);
     leaderFollowingWeight->setSingleStep(5);
     leaderFollowingWeight->setRange(1,1000);
     leaderFollowingWeight->setValue(20);
     leaderFollowingWeight->setScale(0.001);

     slowingDistance = new FloatSlider(orientation,this);
     slowingDistance->setObjectName("slowing_distance");
     slowingDistance->setTickPosition(QSlider::TicksBelow);
     slowingDistance->setTickInterval(5);
     slowingDistance->setSingleStep(5);
     slowingDistance->setRange(1,1000);
     slowingDistance->setValue(20);
     slowingDistance->setScale(0.001);

     timeStep = new FloatSlider(orientation,this);
     timeStep->setObjectName("time_step");
     timeStep->setTickPosition(QSlider::TicksBelow);
     timeStep->setTickInterval(1);
     timeStep->setSingleStep(1);
     timeStep->setRange(50,150);
     timeStep->setValue(100);
     timeStep->setScale(0.00003);

     connect(timeStep,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(slowingDistance,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(leaderFollowingWeight,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(wanderWeight,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(avoidWeight,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(goalWeight,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(alignmentWeight,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(seperationWeight,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(angularVelocity,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(maxSpeed,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(searchingRadius,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(cohesionWeight,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));
     connect(minSeparationDistance,SIGNAL(valueChanged(float)),this,SLOT(triggerValue(float)));

     QGridLayout *slidersLayout = new QGridLayout();
     slidersLayout->addWidget(new QLabel("Time Step:"),0,0);
     slidersLayout->addWidget(timeStep,0,1,1,2);
     slidersLayout->addWidget(new QLabel("Min. Seperation Dist.:"),1,0);
     slidersLayout->addWidget(minSeparationDistance,1,1,1,2);
     slidersLayout->addWidget(new QLabel("Search Radius:"),2,0);
     slidersLayout->addWidget(searchingRadius,2,1,1,2);
     slidersLayout->addWidget(new QLabel("Velocity Limit:"),3,0);
     slidersLayout->addWidget(maxSpeed,3,1,1,2);
     slidersLayout->addWidget(new QLabel("Angular Velocity:"),4,0);
     slidersLayout->addWidget(angularVelocity,4,1,1,2);
     slidersLayout->addWidget(new QLabel("Seperations Weight:"),5,0);
     slidersLayout->addWidget(seperationWeight,5,1,1,2);
     slidersLayout->addWidget(new QLabel("Alignment Weight:"),6,0);
     slidersLayout->addWidget(alignmentWeight,6,1,1,2);
     slidersLayout->addWidget(new QLabel("Cohesion Weight:"),7,0);
     slidersLayout->addWidget(cohesionWeight,7,1,1,2);
     //Need to add avoid and goal target. Preferrably a list of rigid bodies.
     slidersLayout->addWidget(new QLabel("Goal Weight:"),8,0);
     slidersLayout->addWidget(goalWeight,8,1,1,2);
     slidersLayout->addWidget(new QLabel("Avoid Weight:"),9,0);
     slidersLayout->addWidget(avoidWeight,9,1,1,2);
     slidersLayout->addWidget(new QLabel("Wander Weight:"),10,0);
     slidersLayout->addWidget(wanderWeight,10,1,1,2);
     slidersLayout->addWidget(new QLabel("Follow Leader Weight:"),11,0);
     slidersLayout->addWidget(leaderFollowingWeight,11,1,1,2);
     slidersLayout->addWidget(new QLabel("Slowing Distance:"),12,0);
     slidersLayout->addWidget(slowingDistance,12,1,1,2);
     setLayout(slidersLayout);
 }

void FlockingParameterGroup::setValues(RTPSSettings *settings)
{

    timeStep->blockSignals(true);
    minSeparationDistance->blockSignals(true);
    searchingRadius->blockSignals(true);
    maxSpeed->blockSignals(true);
    angularVelocity->blockSignals(true);
    seperationWeight->blockSignals(true);
    alignmentWeight->blockSignals(true);
    cohesionWeight->blockSignals(true);
    goalWeight->blockSignals(true);
    avoidWeight->blockSignals(true);
    wanderWeight->blockSignals(true);
    leaderFollowingWeight->blockSignals(true);
    slowingDistance->blockSignals(true);
    timeStep->setValue(settings->GetSettingAs<float>("time_step","0.003"));
    minSeparationDistance->setValue(settings->GetSettingAs<float>("min_separation_distance","500.0"));
    searchingRadius->setValue(settings->GetSettingAs<float>("searching_radius","500.0"));
    maxSpeed->setValue(settings->GetSettingAs<float>("max_speed","2.0"));
    angularVelocity->setValue(settings->GetSettingAs<float>("angular_velocity","0.00"));
    seperationWeight->setValue(settings->GetSettingAs<float>("seperation_weight","1.0"));
    alignmentWeight->setValue(settings->GetSettingAs<float>("alignment_weight","0.5"));
    cohesionWeight->setValue(settings->GetSettingAs<float>("cohesion_weight","0.5"));
    goalWeight->setValue(settings->GetSettingAs<float>("goal_weight","0.0"));
    avoidWeight->setValue(settings->GetSettingAs<float>("avoid_weight","0.0"));
    wanderWeight->setValue(settings->GetSettingAs<float>("wander_weight","0.5"));
    leaderFollowingWeight->setValue(settings->GetSettingAs<float>("leader_following_weight","0.0"));
    slowingDistance->setValue(settings->GetSettingAs<float>("slowing_distance","0.025"));
    timeStep->blockSignals(false);
    minSeparationDistance->blockSignals(false);
    searchingRadius->blockSignals(false);
    maxSpeed->blockSignals(false);
    angularVelocity->blockSignals(false);
    seperationWeight->blockSignals(false);
    alignmentWeight->blockSignals(false);
    cohesionWeight->blockSignals(false);
    goalWeight->blockSignals(false);
    avoidWeight->blockSignals(false);
    wanderWeight->blockSignals(false);
    leaderFollowingWeight->blockSignals(false);
    slowingDistance->blockSignals(false);
}
}
