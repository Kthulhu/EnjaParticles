#ifndef FLOATSLIDER_H
#define FLOATSLIDER_H
#include <QSlider>

class FloatSlider : public QSlider
{
    Q_OBJECT
public:
    FloatSlider(Qt::Orientation orientation, QWidget* parent = 0);
    void setScale(float scale);
public slots:
    void sliderChange(SliderChange change);
    void setValue(float value);
signals:
    void valueChanged(float value);
private:
    float scale;
};
#endif
