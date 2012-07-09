 #ifndef SPHPARAMETERGROUP_H
 #define SPHPARAMETERGROUP_H

 #include <QGroupBox>

 class QSlider;
 class FloatSlider;
 class QLineEdit;

 class SPHParameterGroup : public QGroupBox
 {
     Q_OBJECT

 public:
     SPHParameterGroup(Qt::Orientation orientation,
                  const QString &title,
                  QWidget *parent = 0);

 signals:
     void valueChanged(const QString& parameterName, const QString& value);

 public slots:
     void setXSPHValue(float value);
     void setGasConstantValue(float value);
     void setViscosityValue(float value);
     void setXSPHValue(const QString& value);
     void setGasConstantValue(const QString& value);
     void setViscosityValue(const QString& value);


 protected slots:
     void triggerValue(int value);
     void triggerValue(float value);
     void triggerValue(const QString& value);

 protected:
     QGroupBox* createVectorInput(const QString& title, QLineEdit* x,QLineEdit* y,QLineEdit* z);
 private:
     FloatSlider* xSPHSlider;
     QLineEdit* xSPHLineEdit;
     FloatSlider* gasConstantSlider;
     QLineEdit* gasConstantLineEdit;
     FloatSlider* viscositySlider;
     QLineEdit* viscosityLineEdit;
     QLineEdit *gravityX,*gravityY,*gravityZ;
 };

 #endif
