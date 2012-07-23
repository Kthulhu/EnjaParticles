#ifndef RIGIDBODYPARAMETERGROUP_H
#define RIGIDBODYPARAMETERGROUP_H

#include <QGroupBox>
#include "rtpsparametergroup.h"
#include "../rtpslib/RTPSSettings.h"
class QSlider;
class FloatSlider;
class QLineEdit;

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
     void setXSPHValue(float value);
     void setGasConstantValue(float value);
     void setViscosityValue(float value);
     void setXSPHValue(const QString& value);
     void setGasConstantValue(const QString& value);
     void setViscosityValue(const QString& value);
     void setValues(RTPSSettings *settings);

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
     QLineEdit *gravityX,*gravityY,*gravityZ;
    };
}

#endif
