#ifndef MYSPINNERWIDGET_H
#define MYSPINNERWIDGET_H

#include <QObject>
#include <QWidget>

#include <QPropertyAnimation>

class MySpinnerWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal a READ a WRITE setA NOTIFY aChanged)
public:
    explicit MySpinnerWidget(QWidget *parent = nullptr);

    QSize minimumSizeHint() const override;
//    QSize sizeHint() const override;

    qreal a() const
    {
        return m_a;
    }
    void setA(qreal a)
    {
        if (qFuzzyCompare(m_a, a))
        {
            return;
        }
        m_a = a;
        emit aChanged(a);
    }
signals:
    void aChanged(qreal);

protected:
    void paintEvent(QPaintEvent *event) override;
    void changeEvent(QEvent *event) override;
//    void resizeEvent(QResizeEvent *event) override;

private:
    qreal m_a;
    QPropertyAnimation *m_anim;
};

#endif // MYSPINNERWIDGET_H
