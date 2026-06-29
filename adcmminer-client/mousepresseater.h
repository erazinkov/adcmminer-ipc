#ifndef MOUSEPRESSEATER_H
#define MOUSEPRESSEATER_H

#include <QObject>
#include <QEvent>

class MousePressEater : public QObject
{
    Q_OBJECT
public:
    MousePressEater(QObject *parent);

protected:
    bool eventFilter(QObject *object, QEvent *event) override;
};

#endif // MOUSEPRESSEATER_H
