 #ifndef SPHPARAMETERGROUP_H
 #define SPHPARAMETERGROUP_H

 #include <QGroupBox>
#include "rtpsparametergroup.h"
#include "../rtpslib/RTPSSettings.h"
 class QSlider;
 class FloatSlider;
 class QComboBox;
 class QLineEdit;

 namespace rtps
 {
 class SPHParameterGroup : public RTPSParameterGroup
 {
     Q_OBJECT

 public:
     SPHParameterGroup(Qt::Orientation orientation,
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
     FloatSlider* xSPHSlider;
     QLineEdit* xSPHLineEdit;
     FloatSlider* gasConstantSlider;
     QLineEdit* gasConstantLineEdit;
     FloatSlider* viscositySlider;
     QLineEdit* viscosityLineEdit;
     FloatSlider* timeStep;
     QComboBox* integrator;
     QLineEdit *gravityX,*gravityY,*gravityZ;
 };
 }

 #endif
