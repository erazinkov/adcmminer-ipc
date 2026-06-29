#include "mousepresseater.h"

MousePressEater::MousePressEater(QObject *parent) : QObject(parent) {

}

bool MousePressEater::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        qDebug("Hovered");
        return true;
    }
    else
    {
        return QObject::eventFilter(object, event);
    }
}
