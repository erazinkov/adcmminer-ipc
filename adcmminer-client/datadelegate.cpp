#include "datadelegate.h"

#include "TFile.h"
#include "TKey.h"

DataDelegate::DataDelegate(QObject *parent)
    : QObject(parent)
{
}

void DataDelegate::histToData(TH1 *hist)
{
    m_data.clear();
    auto nBins{hist->GetNbinsX()};
    QList<QPointF> data;
    data.reserve(nBins);
    for (auto i{1}; i <= nBins; ++i)
    {
        auto x{hist->GetBinCenter(i)};
        auto y{hist->GetBinContent(i)};
        if (y < 0)
        {
            y = 0.0;
        }
        data.emplace_back(x, y);
    }

    if (data.isEmpty())
    {
        return;
    }

    QColor dataColor{m_defaultDataColor};
    QMap<QString, QColor>::const_iterator iter{m_dataColor.find(hist->GetName())};
    if (iter != m_dataColor.end()) {
        dataColor = iter.value();
    }
    m_data.insert(hist->GetName(), data);
    emit(newData(m_data));
}

void DataDelegate::functionToData(TF1 *function)
{
    QList<QPointF> data;
    for (auto i{0}; i <= 8'000; ++i)
    {
        auto x{i};
        auto y{function->Eval(i)};
        if (y < 0)
        {
            y = 0.0;
        }
        data.append(QPointF(x, y));
    }

    if (data.isEmpty())
    {
        return;
    }

    QColor dataColor{m_defaultDataColor};
    QMap<QString, QColor>::const_iterator iter{m_dataColor.find(function->GetName())};
    if (iter != m_dataColor.end()) {
        dataColor = iter.value();
    }
    if (QString(function->GetName()) == "Fit")
    {
        return;
    }
}

const QMap<QString, QList<QPointF> > &DataDelegate::data() const
{
    return m_data;
}


