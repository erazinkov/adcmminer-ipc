#include "piechartwidget.h"
#include <algorithm>

PieChartWidget::PieChartWidget(QWidget *parent)
    : QWidget{parent}
{
    m_chart = new QChart();
    m_chart->setAnimationOptions(QChart::SeriesAnimations);
    m_series = new QPieSeries();
    m_chart->addSeries(m_series);
    m_chartView = new QChartView(m_chart, this);
    m_mainLayout = new QVBoxLayout(this);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_mainLayout->addWidget(m_chartView);

}

void PieChartWidget::setData(const QMap<QString, double> &data)
{
    m_series->clear();
    double s{0.0};
    for (auto i = data.cbegin(), end = data.cend(); i != end; ++i) {
        s += i.value();
    }
    if (s > 0.0) {
        for (auto i = data.cbegin(), end = data.cend(); i != end; ++i) {
            m_series->append(i.key(), 100.0 * i.value() / s);
        }
    }
    m_series->setLabelsVisible(true);
    for (QPieSlice *slice : m_series->slices()) {
        slice->setLabelVisible(true);
        double percentage = (slice->value() / m_series->sum()) * 100.0;
        slice->setLabel(QString("%1 - %2ms %3%").arg(slice->label()).arg(qRound(slice->value() * s / 100.0)).arg(percentage, 0, 'f', 1));
    }
}
