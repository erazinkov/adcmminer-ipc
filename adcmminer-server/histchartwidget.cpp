#include "histchartwidget.h"

#include <QHBoxLayout>
#include <QValueAxis>

HistChartWidget::HistChartWidget(const double xMin, const double xMax, QWidget *parent) : QWidget{parent}
{
    m_sL = {
        "<span><font color='red'>?</font></span>", "<span>?</span>", "<span>?</span>"
    };
    m_chart = new QChart;
    m_chartView = new ChartView(m_chart, this);
    QWidget *header = new QWidget(this);
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    header->setLayout(headerLayout);
    for (auto &s : m_sL) {
        m_headerLabels.push_back(new QLabel(s));
        headerLayout->addWidget(m_headerLabels.last());
    }
    headerLayout->insertStretch(1, 1);
    QVBoxLayout *m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->addWidget(header);
    m_mainLayout->addWidget(m_chartView);
    m_chart->legend()->setVisible(false);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    m_lineSeries = new QLineSeries();
    m_chart->addSeries(m_lineSeries);

    m_axisX = new QValueAxis();
    m_axisX->setRange(xMin, xMax);
    m_axisX->setTickCount(5);
    m_axisX->setLabelFormat("%d");
    m_axisY = new QValueAxis();
    m_axisY->setRange(0, 1.0);
    m_axisY->setTickCount(5);

    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    m_lineSeries->attachAxis(m_axisX);
    m_lineSeries->attachAxis(m_axisY);
}

void HistChartWidget::setHeader(const QStringList &newData)
{
    for (auto i{0}; i < std::min(m_headerLabels.size(), newData.size()); i++) {
        m_headerLabels[i]->setText(newData.at(i));
    }
}

void HistChartWidget::setData(const QString &title, const QList<QPointF> &points)
{
    if (points.isEmpty()) {
        return;
    }
    m_chart->setTitle("");
    auto pointsMaxY = std::max_element(points.constBegin(), points.constEnd(),
            [](const QPointF &a, const QPointF &b) {
                return a.y() < b.y();
            })->y();
    m_lineSeries->replace(points);
    if (m_axisY->max() < pointsMaxY) {
        m_axisY->setRange(m_axisY->min(), pointsMaxY);
    }
    if (qFuzzyCompare(0.0, pointsMaxY)) {
        m_axisY->setRange(0, 1.0);
    }
}
