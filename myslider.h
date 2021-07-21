#ifndef MYSLIDER_H
#define MYSLIDER_H

#include <QObject>
#include <QSlider>
#include <QMouseEvent>
#include <QApplication>
class mySlider: public QSlider
{
    Q_OBJECT
public:
    mySlider(QWidget * parents = nullptr);
protected:
    void mousePressEvent(QMouseEvent *);

signals:
    void sliderClicked(int value);
};

#endif // MYSLIDER_H
