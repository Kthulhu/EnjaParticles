 #ifndef PARTICLEEFFECTPARAMETERGROUP_H
 #define PARTICLEEFFECTPARAMETERGROUP_H

 #include <QGroupBox>

 class QSlider;
 class FloatSlider;
 class QLineEdit;
 class QComboBox;
 class QCheckBox;
 class QRadioButton;
 class QButtonGroup;
 class ParticleEffectParameterGroup : public QGroupBox
 {
     Q_OBJECT

 public:
     ParticleEffectParameterGroup(Qt::Orientation orientation,
                  const QString &title,
                  QWidget *parent = 0);

 signals:
     void valueChanged(const QString& parameterName, const QString& value);

 public slots:
     void setValue(int value);
     void setValue(float value);
     void setValue(const QString& value);

 protected slots:
     void triggerValue(bool value);
     void triggerValue(int value);
     void triggerValue(float value);
     void triggerValue(const QString& value);

 protected:
     QGroupBox* createVectorInput(const QString& title, QLineEdit* x,QLineEdit* y,QLineEdit* z);
 private:
     QComboBox* filterType;
     QCheckBox* thicknessCheck;
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
     QSlider* curvatureFlowIterations;

 };

 #endif
