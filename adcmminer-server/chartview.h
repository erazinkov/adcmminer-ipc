#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QChartView>
#include <QWidget>


class ChartView : public QChartView
{
    Q_OBJECT
public:
    explicit ChartView(QWidget *parent = nullptr);
    explicit ChartView(QChart *chart, QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // CHARTVIEW_H
