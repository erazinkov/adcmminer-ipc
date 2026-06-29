#include "chartview.h"

ChartView::ChartView(QWidget *parent)
    : QChartView(parent)
{
}

ChartView::ChartView(QChart *chart, QWidget *parent)
    : QChartView(chart, parent)
{
}

void ChartView::resizeEvent(QResizeEvent *event)
{
    chart()->setAnimationOptions(QChart::NoAnimation);
    QChartView::resizeEvent(event);
    chart()->setAnimationOptions(QChart::SeriesAnimations);
}
