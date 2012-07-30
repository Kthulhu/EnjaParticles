#include "floatslider.h"
#include <iostream>

//FloatSlider::FloatSlider(Qt::Orientation orientation, QWidget* parent)
//:QSlider(orientation, parent)
//{
//    scale =1.0f;
//}
FloatSlider::FloatSlider(Qt::Orientation orientation, QWidget* parent, float scale)
:QSlider(orientation, parent)
{
    this->scale = scale;
}
void FloatSlider::setValue(float value)
{
    QSlider::setValue(value/scale);
}
void FloatSlider::setScale(float scale)
{
    this->scale=scale;
}
void FloatSlider::sliderChange(SliderChange change)
{
    if(change==QAbstractSlider::SliderValueChange)
    {
         emit valueChanged(value()*scale);
    }
    QSlider::sliderChange(change);
}

