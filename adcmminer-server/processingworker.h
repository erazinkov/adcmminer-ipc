#ifndef PROCESSINGWORKER_H
#define PROCESSINGWORKER_H

#include <QObject>

#include "decoder.h"
#include "datadelegate.h"
#include "calibration.h"

class Enums {
public:
    enum class Type
    {
        TIME,
        AMP,
    };
    Q_ENUM(Type)

    Q_GADGET
    Enums() = delete;
};

class ProcessingWorker : public QObject
{
    Q_OBJECT
public:
    explicit ProcessingWorker(QObject *parent = nullptr);
public slots:
    void doWorkS(const QString &);
    void doWorkReset();
signals:
    void resultReadyTimeCorrectedByAlpha(const QMap<QString, QList<QPointF>> &, const QMap<QString, QStringList> &text);
    void resultReadyEnergyByAlpha(const QMap<QString, QList<QPointF>> &data, const QMap<QString, QStringList> &text);
    void resultReadyProcessing(const QMap<QString, double> &data, double t, const QMap<QString, double> &countersA, const QMap<QString, double> &countersG);
private:
    Decoder *m_decoder;
    DataDelegate *m_dataDelegate;
    HistogramManager *m_histogramManager;
    Calibration *m_calibration;

    void histToPointsTimeCorrectedByAlpha();
    void histToPointsAmpByGamma();

    QVector<QPointF> histToPoints(TH1D *hist);
};

#endif // PROCESSINGWORKER_H
