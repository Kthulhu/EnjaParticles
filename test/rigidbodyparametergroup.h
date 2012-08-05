#ifndef RIGIDBODYPARAMETERGROUP_H
#define RIGIDBODYPARAMETERGROUP_H

#include <QGroupBox>
#include "rtpsparametergroup.h"
#include "../rtpslib/RTPSSettings.h"
class QSlider;
class FloatSlider;
class QComboBox;
class QLineEdit;
class QPushButton;

namespace rtps
{
    class RigidbodyParameterGroup : public RTPSParameterGroup
    {
     Q_OBJECT

    public:
     RigidbodyParameterGroup(Qt::Orientation orientation,
                  const QString &title,
                  QWidget *parent = 0);
    public slots:
     void setValues(RTPSSettings *settings);
     void meshListUpdated(const std::vector<QString>& meshes);
     void addRigidBody();
    signals:
     void addRigidBody(const QString& mesh, float4 pos, float4 vel, float mass);

    private:
     QSlider* subIntervals;
     FloatSlider* velocityLimit;
     FloatSlider* frictionDynamic;
     FloatSlider* frictionStatic;
     FloatSlider* frictionStaticThreshold;
     FloatSlider* penetrationFactor;
     FloatSlider* restitution;
     FloatSlider* timeStep;
     QComboBox* integrator;
     QPushButton* addRigidBodyButton;
     QComboBox* meshSelection;
     QLineEdit *gravityX,*gravityY,*gravityZ;
     QLineEdit *posX,*posY,*posZ;
     QLineEdit *velX,*velY,*velZ;
     QLineEdit *mass;
    };
}

#endif
