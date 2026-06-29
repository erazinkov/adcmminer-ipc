#include "timepeaksfinder.h"

#include <TF1.h>
#include <TList.h>

#include <fstream>
#include <iostream>
#include <sstream>

TimePeaksFinder::TimePeaksFinder(const ChannelMap &map)
{
    _timePeaksPos.resize(map.getIdxsByType(Channel::GAMMA).size());
    for (auto & item : _timePeaksPos)
    {
        item.resize(map.getIdxsByType(Channel::ALPHA).size(), 0.0);
    }
}

const std::vector<std::vector<double> > &TimePeaksFinder::timePeaksPos() const
{
    return _timePeaksPos;
}

void TimePeaksFinder::writePeaksPosToFile(const std::string &fileName)
{
    std::ofstream ofs(fileName);
    if (ofs.is_open())
    {
        for (size_t ig{0}; ig < _timePeaksPos.size(); ++ig)
        {
            for (size_t ia{0}; ia <  _timePeaksPos.at(ig).size(); ++ia)
            {
                ofs << _timePeaksPos.at(ig).at(ia) << " ";
            }
            ofs << std::endl;
        }
        ofs.close();
    }
    else
    {
        std::cerr << "Can't open file " << fileName << std::endl;
    }
}

void TimePeaksFinder::readPeaksPosFromFile(const std::string &fileName)
{
    std::ifstream ifs(fileName);
    if (ifs.is_open())
    {
        for (size_t ig{0}; ig < _timePeaksPos.size(); ++ig)
        {
            std::string line;
            std::getline(ifs, line);
            std::stringstream ss(line);
            double t{0.0};
            for (size_t ia{0}; ia <  _timePeaksPos.at(ig).size(); ++ia)
            {
                ss >> t;
                _timePeaksPos.at(ig).at(ia) = t;
            }
        }
        ifs.close();
    }
    else
    {
        std::cerr << "Can't open file " << fileName << std::endl;
    }
}

void TimePeaksFinder::calculatePeaksPos(std::vector<std::vector<std::shared_ptr<TH1>> > &hists)
{
    gErrorIgnoreLevel = 3'000;
    for (size_t ig{0}; ig < hists.size(); ++ig)
    {
        for (size_t ia{0}; ia <  hists.at(ig).size(); ++ia)
        {
            _timePeaksPos.at(ig).at(ia) = calculatePeakPos(hists.at(ig).at(ia).get());
        }
    }
    gErrorIgnoreLevel = 0;
}

void TimePeaksFinder::calculatePeaksPos(std::vector<std::vector<TH1 *> > hists)
{
    gErrorIgnoreLevel = 3'000;
    for (size_t ig{0}; ig < hists.size(); ++ig)
    {
        for (size_t ia{0}; ia <  hists.at(ig).size(); ++ia)
        {
            _timePeaksPos.at(ig).at(ia) = calculatePeakPos(hists.at(ig).at(ia));
        }
    }
    gErrorIgnoreLevel = 0;
}

double TimePeaksFinder::calculatePeakPos(TH1 *hist)
{
//    hist->Rebin();
    auto timePeakPos{0.0};

    auto binMax{hist->GetMaximumBin()};
    auto xMax{hist->GetBinCenter(hist->GetBin(binMax))};
    auto rcAmp{hist->GetBinContent(hist->GetXaxis()->FindBin(xMax - 25.0))};
    auto obPeakAmp{hist->GetBinContent(binMax) - rcAmp};
    auto snPeakAmp{0.5 * obPeakAmp};

    auto ff = [] (double *x, double *par) {
       double arg_1{0.0}, arg_2{0.0}, arg_3{0.0}, arg_4{0.0};
       if (par[2] != 0.0 && par[5] != 0.0 && par[8] != 0.0)
       {
           arg_1 = ( x[0] - par[1] ) / par[2];
           arg_2 = ( x[0] - ( par[1] + par[4] ) ) / par[5];
           arg_3 = ( x[0] - ( par[1] + par[7] ) ) / par[8];
           arg_4 = x[0];
       }

       double fitval{
           par[0] * TMath::Exp( -0.5 * arg_1 * arg_1 ) +
           par[3] * TMath::Exp( -0.5 * arg_2 * arg_2 ) +
           par[6] * TMath::Exp( -0.5 * arg_3 * arg_3 ) +
           par[9] + par[10] * arg_4
       };

       return fitval;
   };

    TF1 *f{new TF1("f", ff, xMax - 15.0, xMax + 25.0, 11)};

    f->SetParameter(0, obPeakAmp);
    f->SetParameter(1, xMax);
    f->SetParameter(2, 0.5 * ( 1.5 + 3.0 ));
    f->SetParameter(6, 0.05 * obPeakAmp);
    f->SetParameter(7, -10.0);
    f->SetParameter(8, 2.5);
    f->SetParameter(9, rcAmp);
    f->FixParameter(10, 0.0);

    f->SetParLimits(1, 0.9 * xMax, 1.1 * xMax);
    f->SetParLimits(2, 1.5, 3.0);
    f->SetParLimits(3, 0.0, snPeakAmp);
    f->SetParLimits(4, 5.0, 20.0);
    f->SetParLimits(5, 2.0, 7.0);
    f->SetParLimits(6, 0.0, 0.25 * obPeakAmp);
    f->SetParLimits(7, -15.0, -7.5);
    f->SetParLimits(8, 2.25, 2.75);

    hist->GetXaxis()->SetRangeUser(f->GetParameter(1) - 40.0, f->GetParameter(1) + 25.0);

    hist->Fit(f, "RQ");

    auto fff = [](double *x, double *par){
        double arg{0};
        if (par[2] != 0.0)
        {
            arg = ( x[0] - par[1] ) / par[2];
        }
        double fitval{par[0] * TMath::Exp(-0.5 * arg * arg) + par[3] + par[4] * x[0]};
        return fitval;
    };

    TF1 *fOb{new TF1("fOb", fff, xMax - 15.0, xMax + 25.0, 5)};
    fOb->SetParameters(f->GetParameter(0),
                       f->GetParameter(1),
                       f->GetParameter(2),
                       f->GetParameter(9),
                       f->GetParameter(10));
    fOb->SetLineColor(kGreen);
    TF1 *fB{new TF1("fB", fff, xMax - 15.0, xMax + 25.0, 5)};
    fB->SetParameters(f->GetParameter(6),
                       f->GetParameter(1) + f->GetParameter(7),
                       f->GetParameter(8),
                       f->GetParameter(9),
                       f->GetParameter(10));
    fB->SetLineColor(kMagenta);
    TF1 *fSn{new TF1("fSn", fff, xMax - 15.0, xMax + 25.0, 5)};
    fSn->SetParameters(f->GetParameter(3),
                       f->GetParameter(1) + f->GetParameter(4),
                       f->GetParameter(5),
                       f->GetParameter(9),
                       f->GetParameter(10));
    fSn->SetLineColor(kBlue);

//    hist->GetListOfFunctions()->Add(fOb);
//    hist->GetListOfFunctions()->Add(fB);
//    hist->GetListOfFunctions()->Add(fSn);

    timePeakPos = f->GetParameter(1);
    timePeakPos = xMax;

    delete f;
    f = nullptr;

    return timePeakPos;
}
