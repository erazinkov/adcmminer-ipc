#ifndef HISTCHARTWIDGET_H
#define HISTCHARTWIDGET_H

#include <QObject>
#include <QWidget>
#include <QChart>
#include <QLabel>
#include <QLineSeries>
#include <QValueAxis>

#include "chartview.h"

class HistChartWidget : public QWidget
{
    Q_OBJECT
public:
    HistChartWidget(const double xMin, const double xMax, QWidget *parent = nullptr);


public slots:
    void setHeader(const QStringList &newData);
    void setData(const QString &title, const QList<QPointF> &points);
private:
    QChart *m_chart;
    ChartView *m_chartView;

    QStringList m_sL;
    QList<QLabel *> m_headerLabels;
    QLineSeries *m_lineSeries;

    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
};

#endif // HISTCHARTWIDGET_H
