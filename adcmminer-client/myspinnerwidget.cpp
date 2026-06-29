#include "myspinnerwidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>

#include <cmath>

MySpinnerWidget::MySpinnerWidget(QWidget *parent)
    : QWidget{parent}
{
    setBackgroundRole(QPalette::Base);
    // setAttribute(Qt::WA_NoSystemBackground);
    // setAttribute(Qt::WA_TransparentForMouseEvents);
    m_anim = new QPropertyAnimation(this, "a");
    m_anim->setDuration(5'000);
    m_anim->setKeyValueAt(0.0, 0.0);
    m_anim->setKeyValueAt(1.0, 360.0);
//    m_anim->setEasingCurve(QEasingCurve::OutInElastic);
//    m_anim->setEasingCurve(QEasingCurve::InOutBounce);
    m_anim->setLoopCount(-1);

    connect(this, &MySpinnerWidget::aChanged, [this](){
        this->update();
    });

    m_anim->start();


}

void MySpinnerWidget::paintEvent(QPaintEvent *)
{
    if (this->parentWidget()) {
        this->setGeometry(this->parentWidget()->rect());
    }
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);



//    int side = qMin(width(), height());
//    int radius = side / 2;
    QPointF center(width() / 2, height() / 2);
//    painter.translate(center);
//    painter.scale(radius / 29.5, radius / 29.5);


    auto side{qMin(width(), height())};
    auto radius{side / 2};
    auto width{5.0 * radius * 0.05};
    painter.translate(center);

    auto rect{QRectF(-radius + width, -radius + width, 2.0 * (radius - width), 2.0 * (radius - width))};
    QConicalGradient gradient;
    gradient.setCenter(rect.center());
    gradient.setAngle(0);
    gradient.setColorAt(0, Qt::gray);
    gradient.setColorAt(1.0, Qt::transparent);

    painter.rotate(m_a);

    QPen pen(QBrush(gradient), width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);
    painter.drawArc(rect, 0, 360 * 16);
    auto w1{width * 0.05};
    QPen pen1(QBrush(Qt::gray), w1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setBrush(QBrush(Qt::white));
    painter.setPen(pen1);
    painter.drawEllipse(QRectF(radius - 1.5 * width + 0.5 * w1, -width / 2 + 0.5 * w1, width -  w1, width  - w1));


}

void MySpinnerWidget::changeEvent(QEvent *event)
{
//    if (event->type() == QEvent::EnabledChange) {
//        if (isEnabled())
//        {
//            m_bgColor = Qt::red;
//            m_tColor = Qt::black;
//            m_bColor = Qt::yellow;
//            m_anim->start();
//        }
//        else
//        {
//            m_bgColor = Qt::transparent;
//            m_tColor = Qt::gray;
//            m_bColor = m_tColor.lighter();
//            m_anim->stop();
//        }
//    }
    QWidget::changeEvent(event);
}

QSize MySpinnerWidget::minimumSizeHint() const
{
    return QSize(10, 10);
}

//QSize MySpinnerWidget::sizeHint() const
//{
//    return QSize(20, 20);
//}

//void MySpinnerWidget::resizeEvent(QResizeEvent *event)
//{
//    int side = qMin(event->size().width(), event->size().height());
//    resize(side, side);
//}
