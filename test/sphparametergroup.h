 #ifndef SPHPARAMETERGROUP_H
 #define SPHPARAMETERGROUP_H

 #include <QGroupBox>

 class QSlider;
 class QLineEdit;
 class SPHParameterGroup : public QGroupBox
 {
     Q_OBJECT

 public:
     SPHParameterGroup(Qt::Orientation orientation,
                  const QString &title,
                  QWidget *parent = 0);

 signals:
     void valueChanged(int value);
     void valueChanged(const QString& value);

 public slots:
     void setValue(int value);
     void setValue(const QString& value);

protected slots:
void triggerValue(int value);
void triggerValue(const QString& value);

protected:
QGroupBox* createVectorInput(const QString& title, QLineEdit* x,QLineEdit* y,QLineEdit* z);
 private:
     QSlider* xSPHSlider;
     QLineEdit *gravityX,*gravityY,*gravityZ;
 };

 #endif
