 #ifndef PARTICLEEFFECTPARAMETERGROUP_H
 #define PARTICLEEFFECTPARAMETERGROUP_H

 #include <QGroupBox>
#include "rtpsparametergroup.h"

 class QSlider;
 class FloatSlider;
 class QLineEdit;
 class QComboBox;
 class QCheckBox;
 class QRadioButton;
 class QButtonGroup;
 namespace rtps
 {
 class ParticleEffectParameterGroup : public RTPSParameterGroup
 {
     Q_OBJECT

 public:
     ParticleEffectParameterGroup(Qt::Orientation orientation,
                  const QString &title,
                  QWidget *parent = 0);

 public slots:
    virtual void setValues(RTPSSettings* settings);
 private:
     QComboBox* filterType;
     QCheckBox* thicknessCheck;
     QCheckBox* velocityCheck;
     QRadioButton* renderNormal;
     QRadioButton* renderDepth;
     QRadioButton* renderDepthSmoothed;
     QRadioButton* renderThickness;
     QRadioButton* renderComposite;
     QGroupBox* renderButtonGroup;
     FloatSlider* pointScaleSlider;
     FloatSlider* blurRadius;
     FloatSlider* bilateralRange;
     FloatSlider* thicknessGamma;
     FloatSlider* curvatureFlowDT;
     FloatSlider* velocityScale;
     QSlider* curvatureFlowIterations;

 };
}
 #endif
