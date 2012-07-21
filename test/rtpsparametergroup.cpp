#include <QtGui>
#include <QLineEdit>
#include <QObject>
#include "rtpsparametergroup.h"
#include "../rtpslib/debug.h"
namespace rtps
{
RTPSParameterGroup::RTPSParameterGroup(const QString &title,
                        QWidget *parent)
 : QGroupBox(title, parent)
{

}
QGroupBox* RTPSParameterGroup::createVectorInput(const QString& title, QLineEdit* x,QLineEdit* y,QLineEdit* z)
{
     QGroupBox* box = new QGroupBox(title);
     QBoxLayout* boxLayout = new QBoxLayout(QBoxLayout::LeftToRight);
     boxLayout->addWidget(new QLabel("X:"));
     boxLayout->addWidget(x);
     boxLayout->addWidget(new QLabel("Y:"));
     boxLayout->addWidget(y);
     boxLayout->addWidget(new QLabel("Z:"));
     boxLayout->addWidget(z);
     box->setLayout(boxLayout);
     return box;
}
void RTPSParameterGroup::triggerValue(bool value)
{
    //std::cout<<"name = "<<(const char*)this->sender()->objectName().toAscii().data() <<" value = "<<(value?"enabled":"disabled")<<std::endl;
    emit valueChanged(this->sender()->objectName(),QString(value?"1":"0"));
}
void RTPSParameterGroup::triggerValue(int value)
{
    //std::cout<<"name = "<<(const char*)this->sender()->objectName().toAscii().data() <<" value = "<<value<<std::endl;
    emit valueChanged(this->sender()->objectName(),QString::number(value));
}
void RTPSParameterGroup::triggerValue(float value)
{
    //std::cout<<"name = "<<(const char*)this->sender()->objectName().toAscii().data() <<" value = "<<value<<std::endl;
    emit valueChanged(this->sender()->objectName(),QString::number(value));
}
void RTPSParameterGroup::triggerValue(const QString& value)
{
    //std::cout<<"name = "<<(const char*)this->sender()->objectName().toAscii().data() <<" value = "<<value.toAscii().data()<<std::endl;
    emit valueChanged(this->sender()->objectName(),value);
}
void RTPSParameterGroup::triggerVectorValue(const QString& value)
{
    QString name = this->sender()->objectName();
    QString component = name.section('_',-1); //x y or z
    name.truncate(name.lastIndexOf('_'));
    dout<<"componenet = "<<component.toAscii().data()<<" property name = "<<name.toAscii().data()<<std::endl;
    QString xValue,yValue,zValue;
    //char findComponents[2];
    if(component=="x")
    {
        xValue=value;
        yValue=this->findChild<QLineEdit*>(name+"_y")->displayText();
        zValue=this->findChild<QLineEdit*>(name+"_z")->displayText();
    }
    else if(component=="y")
    {
        xValue=this->findChild<QLineEdit*>(name+"_x")->displayText();
        yValue=value;
        zValue=this->findChild<QLineEdit*>(name+"_z")->displayText();
    }
    else
    {
        xValue=this->findChild<QLineEdit*>(name+"_x")->displayText();
        yValue=this->findChild<QLineEdit*>(name+"_y")->displayText();
        zValue=value;
    }
    emit valueChanged(name,xValue+" "+yValue+" "+zValue+" 0.0");
}
}
