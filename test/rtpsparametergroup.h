#ifndef RTPSPARAMETERGROUP_H
#define RTPSPARAMETERGROUP_H

#include <QGroupBox>
#include "../rtpslib/RTPSSettings.h"
#include <QLineEdit>
namespace rtps
{
 class RTPSParameterGroup : public QGroupBox
 {
     Q_OBJECT

 public:
     RTPSParameterGroup(const QString &title,
                  QWidget *parent);
 signals:
     void valueChanged(const QString& parameterName, const QString& value);

 public slots:
     //virtual void setValue(const QString& param,int value) = 0;
     //virtual void setValue(const QString& param,float value) = 0;
     //virtual void setValue(const QString& param,const QString& value) = 0;
     virtual void setValues(RTPSSettings* settings) = 0;

 protected slots:
     virtual void triggerValue(bool value);
     virtual void triggerValue(int value);
     virtual void triggerValue(float value);
     virtual void triggerValue(const QString& value);
     virtual void triggerVectorValue(const QString& value);

 protected:
     QGroupBox* createVectorInput(const QString& title, QLineEdit* x,QLineEdit* y,QLineEdit* z);
 };
}
 #endif
