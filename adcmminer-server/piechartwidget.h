#ifndef PIECHARTWIDGET_H
#define PIECHARTWIDGET_H

#include <QObject>
#include <QWidget>

#include <QChart>
#include <QChartView>
#include <QPieSeries>
#include <QVBoxLayout>

class PieChartWidget : public QWidget
{
    Q_OBJECT
public:
    PieChartWidget(QWidget *parent = nullptr);
public slots:
    void setData(const QMap<QString, double> &);
private:
    QChart *m_chart;
    QChartView *m_chartView;
    QPieSeries *m_series;
    QVBoxLayout *m_mainLayout;
};

#endif // PIECHARTWIDGET_H
