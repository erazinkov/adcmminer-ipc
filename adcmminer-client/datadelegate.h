#ifndef DATADELEGATE_H
#define DATADELEGATE_H

#include <QObject>
#include <QMap>
#include <QPointF>
#include <QDebug>
#include <QtCharts/QtCharts>

#include "TH1.h"
#include "TF1.h"

class DataDelegate : public QObject
{
    Q_OBJECT

signals:
    void newData(QMap<QString, QList<QPointF>>);

public:
    DataDelegate(QObject *parent = nullptr);
    void histToData(TH1 *);
    void functionToData(TF1 *);
    const QMap<QString, QList<QPointF> > &data() const;

private:
    QMap<QString, QList<QPointF>> m_data;

    QColor m_defaultDataColor{QColor::fromRgb(192, 192, 192)};
    QMap<QString,  QColor> m_dataColor
    {
        { "Data", QColor::fromRgb(0, 0, 0), },
        { "Bg", QColor::fromRgb(64, 64, 64), },
        { "Fit", QColor::fromRgb(0, 0, 255), },
        { "Al", QColor::fromRgb(255, 0, 0), },
        { "C", QColor::fromRgb(255, 128, 0), },
        { "Ca", QColor::fromRgb(255, 255, 0), },
        { "Cu", QColor::fromRgb(255, 0, 255), },
        { "Fe", QColor::fromRgb(128, 255, 0), },
        { "K", QColor::fromRgb(0, 255, 0), },
        { "Mg", QColor::fromRgb(0, 255, 128), },
        { "Ni", QColor::fromRgb(0, 255, 127), },
        { "O", QColor::fromRgb(0, 255, 255), },
        { "Si", QColor::fromRgb(0, 128, 255), },
        { "S", QColor::fromRgb(0, 0, 255), },
        { "Ti", QColor::fromRgb(127, 255, 0), },
    };
};

#endif // DATADELEGATE_H
