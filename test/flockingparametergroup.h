#ifndef FLOCKINGPARAMETERGROUP_H
#define FLOCKINGPARAMETERGROUP_H

#include <QGroupBox>
#include "rtpsparametergroup.h"
#include "../rtpslib/RTPSSettings.h"
class QSlider;
class FloatSlider;
class QComboBox;
class QLineEdit;

namespace rtps
{
    class FlockingParameterGroup : public RTPSParameterGroup
    {
        Q_OBJECT

        public:
             FlockingParameterGroup(Qt::Orientation orientation,
                          const QString &title,
                          QWidget *parent = 0);
        public slots:
             void setValues(RTPSSettings *settings);

        private:
             FloatSlider* minSeparationDistance;
             FloatSlider* searchingRadius;
             FloatSlider* maxSpeed;
             FloatSlider* angularVelocity;
             FloatSlider* seperationWeight;
             FloatSlider* alignmentWeight;
             FloatSlider* cohesionWeight;
             FloatSlider* goalWeight;
             FloatSlider* avoidWeight;
             FloatSlider* wanderWeight;
             FloatSlider* leaderFollowingWeight;
             FloatSlider* slowingDistance;
             //FloatSlider* leaderIndex;
             //FloatSlider* target;
             FloatSlider* timeStep;
             QComboBox* integrator;

    };
}

#endif
