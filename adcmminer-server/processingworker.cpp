#include "processingworker.h"

#include "constants.h"

ProcessingWorker::ProcessingWorker(QObject *parent)
    : QObject{parent}
{
    m_decoder = new Decoder;
    m_histogramManager = new HistogramManager(AppConstants::MAX_GAMMA_NUMBER, AppConstants::MAX_ALPHA_NUMBER);
    m_calibration = new Calibration(m_histogramManager);
    m_dataDelegate = new DataDelegate;
}

void ProcessingWorker::doWorkS(const QString &path)
{
    QMap<QString, double> m;
    auto start = std::chrono::steady_clock::now();
    m_decoder->process_o(path.toStdString());
    if (m_decoder->events_o().empty() || qFuzzyCompare(m_decoder->time(), 0.0) || m_decoder->counters().empty()) {
        return;
    }
    auto stop = std::chrono::steady_clock::now();
    m.insert(tr("Decoding"), std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count());
    start = std::chrono::steady_clock::now();
    m_calibration->setNewData_o(m_decoder->events_o(), m_decoder->channels(), m_decoder->time(), m_decoder->counters());
    m_calibration->process();
    stop = std::chrono::steady_clock::now();
    m.insert(tr("Calibration"), std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count());
    start = std::chrono::steady_clock::now();
    histToPointsTimeCorrectedByAlpha();
    histToPointsAmpByGamma();
    stop = std::chrono::steady_clock::now();
    QMap<QString, double> countersA;
    for (const auto& [key, value] : m_calibration->countersA()) {
        countersA[QString::number(key)] = value;
    }
    QMap<QString, double> countersG;
    for (const auto& [key, value] : m_calibration->countersG()) {
        countersG[QString::number(key)] = value;
    }
    emit resultReadyProcessing(m, m_calibration->time(), countersA, countersG);
}

void ProcessingWorker::doWorkReset()
{
    m_calibration->resetData();
    m_histogramManager->resetAll();
    histToPointsTimeCorrectedByAlpha();
    histToPointsAmpByGamma();
    emit resultReadyProcessing({}, m_calibration->time(), {}, {});
}

QVector<QPointF> ProcessingWorker::histToPoints(TH1D *hist) {
    QVector<QPointF> points;
    if (!hist) return points;
    for (int bin = 1; bin <= hist->GetNbinsX(); ++bin) {
        double x = hist->GetBinCenter(bin);
        double y = hist->GetBinContent(bin);
        points.append(QPointF(x, y));
    }
    return points;
}

void ProcessingWorker::histToPointsTimeCorrectedByAlpha()
{

    QMap<QString, QList<QPointF>> data;
    QMap<QString, QStringList> text;
    for (ulong i{0}; i < m_decoder->channels().a.size(); ++i) {
        TH1 *h;
        h = m_histogramManager->histsTimeCorrectedByAlpha()[i];
        if (h) {
            m_dataDelegate->histToData(h);
            for (auto j = m_dataDelegate->data().cbegin(), end = m_dataDelegate->data().cend(); j != end; ++j)
            {
                data.insert(j.key(), j.value());
                text.insert(j.key(), {
                                QString("<span><font color='red'>%1</font></span>").arg(i)
                            });
            }
            h = nullptr;
        }

    }
    emit resultReadyTimeCorrectedByAlpha(data, text);
}

void ProcessingWorker::histToPointsAmpByGamma()
{
//    QMap<QString, QList<QPointF>> data;
//    QMap<QString, QString> text;
//    for (ulong i{0}; i < m_decoder->channels().g.size(); ++i) {
//        TH1 *h;
//        h = m_calibration->histogramManager->histsEnergyByGamma()[i];
//        m_dataDelegate->histToData(h);
//        for (auto j = m_dataDelegate->data().cbegin(), end = m_dataDelegate->data().cend(); j != end; ++j)
//        {
//            data.insert(j.key(), j.value());
//            text.insert(j.key(), QString("<b>%1</b><br/>").arg(qRound(h->Integral())));

//        }
//    }

//    emit resultReadyAmpByGamma(data, text);

    QMap<QString, QList<QPointF>> data;
    QMap<QString, QStringList> text;
    double s{0.0};
    for (ulong i{0}; i < m_decoder->channels().a.size(); ++i) {
        TH1 *h;
        h = m_histogramManager->histsEnergyByAlpha()[i];
        s += h->Integral();
        h = nullptr;
    }

    for (ulong i{0}; i < m_decoder->channels().a.size(); ++i) {
        TH1 *h;
        h = m_histogramManager->histsEnergyByAlpha()[i];
        if (h) {
            m_dataDelegate->histToData(h);
            for (auto j = m_dataDelegate->data().cbegin(), end = m_dataDelegate->data().cend(); j != end; ++j) {
                data.insert(j.key(), j.value());
                text.insert(j.key(), {
                                QString("<span><font color='red'>%1</font></span>").arg(i),
                                QString("<span>%1</span>").arg(qRound(h->Integral())),
                                QString("<span>%1%</span>").arg(qRound(100.0 * h->Integral() / (s < 1.0 ? 1.0 : s )))
                            });
                h = nullptr;
            }
        }

    }

    emit resultReadyEnergyByAlpha(data, text);
}
