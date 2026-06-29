#ifndef BARCHARTWIDGET_H
#define BARCHARTWIDGET_H

#include <QWidget>
#include <QChart>
#include <QChartView>
#include <QBarSeries>
#include <QVBoxLayout>
#include <QValueAxis>
#include <QBarCategoryAxis>


class BarChartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BarChartWidget(const QString &title, QWidget *parent = nullptr);

public slots:
    void setData(const QMap<QString, double> &);

private:
    QChart *m_chart;
    QChartView *m_chartView;
    QBarSeries *m_series;
    QVBoxLayout *m_mainLayout;

    QBarCategoryAxis *m_axisX;
    QValueAxis *m_axisY;
    QStringList m_categories;
    QString m_title;
};

#endif // BARCHARTWIDGET_H
