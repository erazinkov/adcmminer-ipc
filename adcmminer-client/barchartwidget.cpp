#include "barchartwidget.h"

#include <QBarSet>

BarChartWidget::BarChartWidget(const QString &title, QWidget *parent)
    :  QWidget{parent}, m_title{title}
{
    m_chart = new QChart();
    m_chart->setAnimationOptions(QChart::SeriesAnimations);

    m_series = new QBarSeries();
    m_chart->addSeries(m_series);

    QBarSet *s = new QBarSet(m_title);
    m_series->append(s);
    m_chartView = new QChartView(m_chart, this);
    m_mainLayout = new QVBoxLayout(this);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_mainLayout->addWidget(m_chartView);

    m_axisX = new QBarCategoryAxis();
    m_axisX->append(QStringList());
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_series->attachAxis(m_axisX);

    m_axisY = new QValueAxis();
    m_axisY->setRange(0.0, 1.0);
    m_axisY->setLabelFormat("%.0f");
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_series->attachAxis(m_axisY);
}

void BarChartWidget::setData(const QMap<QString, double> &data)
{
    if (data.isEmpty()) {
        return;
    }
    QStringList m;
    QBarSet *s = new QBarSet(m_title);

    QList<QPair<QString, double>> dataList;
    for (auto it = data.begin(); it != data.end(); ++it) {
        dataList.append(qMakePair(it.key(), it.value()));
    }
    std::sort(dataList.begin(), dataList.end(), [](const QPair<QString, double>& a, const QPair<QString, double>& b) {
                  return a.first.toInt() < b.first.toInt();
              });
    for (const auto& dataListItem : dataList) {
        s->append(dataListItem.second);
        m.append(dataListItem.first);
    }

    m_series->clear();
    m_series->append(s);

    m_axisX->clear();
    m_axisX->append(m);
    double maxY = *std::max_element(data.begin(), data.end(), [](double a, double b) { return a < b; });
    if (m_axisY->max() < maxY) {
        m_axisY->setRange(m_axisY->min(), maxY);
    }
    if (qFuzzyCompare(0.0, maxY)) {
        m_axisY->setRange(0.0, 1.0);
    }

}
