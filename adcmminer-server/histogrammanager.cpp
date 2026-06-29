#include "histogrammanager.h"

#include <sstream>

#include <iostream>

#include <TCanvas.h>
#include <TError.h>
#include <TFile.h>

HistogramManager::HistogramManager(const int &gammaNumber, const int &alphaNumber, std::optional<std::string> outputDirectory) : gammaNumber_{gammaNumber}, alphaNumber_{alphaNumber}, outputDirectory_{outputDirectory}
{
    histsAmpByGammaAlphaSg_.resize(gammaNumber_);
    for (auto ig{0}; ig < gammaNumber_; ig++) {
        histsAmpByGammaAlphaSg_[ig].resize(alphaNumber_);
        for (auto ia{0}; ia < alphaNumber_; ia++) {
            std::string name{Form("hist_amp_by_gamma_sg_%d_alpha_%d", ig, ia)};
            std::string title{Form("hist_amp_by_gamma_sg_%d_alpha_%d", ig, ia)};
            TH1D *h = new TH1D(name.c_str(), title.c_str(), BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL);
            histsAmpByGammaAlphaSg_[ig][ia] = h;
        }
    }
    histsAmpByGammaAlphaBg_.resize(gammaNumber_);
    for (auto ig{0}; ig < gammaNumber_; ig++) {
        histsAmpByGammaAlphaBg_[ig].resize(alphaNumber_);
        for (auto ia{0}; ia < alphaNumber_; ia++) {
            std::string name{Form("hist_amp_by_gamma_bg_%d_alpha_%d", ig, ia)};
            std::string title{Form("hist_amp_by_gamma_bg_%d_alpha_%d", ig, ia)};
            TH1D *h = new TH1D(name.c_str(), title.c_str(), BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL);
            histsAmpByGammaAlphaBg_[ig][ia] = h;
        }
    }

    histsAmpByGammaAlphaRc_.resize(gammaNumber_);
    for (auto ig{0}; ig < gammaNumber_; ig++) {
        histsAmpByGammaAlphaRc_[ig].resize(alphaNumber_);
        for (auto ia{0}; ia < alphaNumber_; ia++) {
            std::string name{Form("hist_amp_by_gamma_rc_%d_alpha_%d", ig, ia)};
            std::string title{Form("hist_amp_by_gamma_rc_%d_alpha_%d", ig, ia)};
            TH1D *h = new TH1D(name.c_str(), title.c_str(), BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL);
            histsAmpByGammaAlphaRc_[ig][ia] = h;
        }
    }

    histsTimeByGammaAlpha_.resize(gammaNumber_);
    for (auto ig{0}; ig < gammaNumber_; ig++) {
        histsTimeByGammaAlpha_[ig].resize(alphaNumber_);
        for (auto ia{0}; ia < alphaNumber_; ia++) {
            std::string name{Form("hist_time_by_gamma_%d_alpha_%d", ig, ia)};
            std::string title{Form("hist_time_by_gamma_%d_alpha_%d", ig, ia)};
            TH1D *h = new TH1D(name.c_str(), title.c_str(), BINS_TIME, XLOW_TIME, XUP_TIME);
            histsTimeByGammaAlpha_[ig][ia] = h;
        }
    }

    histsAmpByGamma_.resize(gammaNumber_);
    for (auto ig{0}; ig < gammaNumber_; ig++) {
        std::string name{Form("hist_amp_by_gamma_%d", ig)};
        std::string title{Form("hist_amp_by_gamma_%d", ig)};
        TH1D *h = new TH1D(name.c_str(), title.c_str(), BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL);
        histsAmpByGamma_[ig] = h;
    }
    histsAmpByGammaRc_.resize(gammaNumber_);
    for (auto ig{0}; ig < gammaNumber_; ig++) {
        std::string name{Form("hist_amp_by_gamma_rc_%d", ig)};
        std::string title{Form("hist_amp_by_gamma_rc_%d", ig)};
        TH1D *h = new TH1D(name.c_str(), title.c_str(), BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL);
        histsAmpByGammaRc_[ig] = h;
    }

    histsAmpByAlpha_.resize(alphaNumber_);
    for (auto ia{0}; ia < alphaNumber_; ia++) {
        std::string name{Form("hist_amp_by_alpha_%d", ia)};
        std::string title{Form("hist_amp_by_alpha_%d", ia)};
        TH1D *h = new TH1D(name.c_str(), title.c_str(), BINS_CHANNEL, XLOW_CHANNEL, XUP_CHANNEL);
        histsAmpByAlpha_[ia] = h;
    }
    histsTimeCorrectedByGammaAlpha_.resize(gammaNumber_);
    for (auto ig{0}; ig < gammaNumber_; ig++) {
        histsTimeCorrectedByGammaAlpha_[ig].resize(alphaNumber_);
        for (auto ia{0}; ia < alphaNumber_; ia++) {
            std::string name{Form("hist_time_corrected_by_gamma_%d_alpha_%d", ig, ia)};
            std::string title{Form("hist_time_corrected_by_gamma_%d_alpha_%d", ig, ia)};
            TH1D *h = new TH1D(name.c_str(), title.c_str(), BINS_TIME, XLOW_TIME, XUP_TIME);
            histsTimeCorrectedByGammaAlpha_[ig][ia] = h;
        }
    }
    histsTimeCorrectedByAlpha_.resize(alphaNumber_);
    for (auto ia{0}; ia < alphaNumber_; ia++) {
        std::string name{Form("hist_time_corrected_by_alpha_%d", ia)};
        std::string title{Form("hist_time_corrected_by_alpha_%d", ia)};
        TH1D *h = new TH1D(name.c_str(), title.c_str(), BINS_TIME, XLOW_TIME, XUP_TIME);
        histsTimeCorrectedByAlpha_[ia] = h;
    }

    histsEnergyByGammaAlphaSg_.resize(gammaNumber_);
    for (auto ig{0}; ig < gammaNumber_; ig++) {
        histsEnergyByGammaAlphaSg_[ig].resize(alphaNumber_);
        for (auto ia{0}; ia < alphaNumber_; ia++) {
            std::string name{Form("hist_energy_by_gamma_sg_%d_alpha_%d", ig, ia)};
            std::string title{Form("hist_energy_by_gamma_sg_%d_alpha_%d", ig, ia)};
            TH1D *h = new TH1D(name.c_str(), title.c_str(), BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY);
            histsEnergyByGammaAlphaSg_[ig][ia] = h;
        }
    }
    histsEnergyByGammaAlphaBg_.resize(gammaNumber_);
    for (auto ig{0}; ig < gammaNumber_; ig++) {
        histsEnergyByGammaAlphaBg_[ig].resize(alphaNumber_);
        for (auto ia{0}; ia < alphaNumber_; ia++) {
            std::string name{Form("hist_energy_by_gamma_bg_%d_alpha_%d", ig, ia)};
            std::string title{Form("hist_energy_by_gamma_bg_%d_alpha_%d", ig, ia)};
            TH1D *h = new TH1D(name.c_str(), title.c_str(), BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY);
            histsEnergyByGammaAlphaBg_[ig][ia] = h;
        }
    }
    histsEnergyByGamma_.resize(gammaNumber_);
    for (auto ig{0}; ig < gammaNumber_; ig++) {
        std::string name{Form("hist_energy_by_gamma_%d", ig)};
        std::string title{Form("hist_energy_by_gamma_%d", ig)};
        TH1D *h = new TH1D(name.c_str(), title.c_str(), BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY);
        histsEnergyByGamma_[ig] = h;
    }
    histsEnergyByAlpha_.resize(alphaNumber_);
    for (auto ia{0}; ia < alphaNumber_; ia++) {
        std::string name{Form("hist_energy_by_alpha_%d", ia)};
        std::string title{Form("hist_energy_by_alpha_%d", ia)};
        TH1D *h = new TH1D(name.c_str(), title.c_str(), BINS_ENERGY, XLOW_ENERGY, XUP_ENERGY);
        histsEnergyByAlpha_[ia] = h;
    }
}

HistogramManager::~HistogramManager()
{
    for (auto ig{0}; ig < gammaNumber_; ig++) {
        for (auto ia{0}; ia < alphaNumber_; ia++) {
            delete histsTimeByGammaAlpha_[ig][ia];
            histsTimeByGammaAlpha_[ig][ia] = nullptr;

            delete histsAmpByGammaAlphaSg_[ig][ia];
            histsAmpByGammaAlphaSg_[ig][ia] = nullptr;
            delete histsAmpByGammaAlphaBg_[ig][ia];
            histsAmpByGammaAlphaBg_[ig][ia] = nullptr;
            delete histsAmpByGammaAlphaRc_[ig][ia];
            histsAmpByGammaAlphaRc_[ig][ia] = nullptr;
            delete histsEnergyByGammaAlphaSg_[ig][ia];
            histsEnergyByGammaAlphaSg_[ig][ia] = nullptr;
            delete histsEnergyByGammaAlphaBg_[ig][ia];
            histsEnergyByGammaAlphaBg_[ig][ia] = nullptr;
        }
    }
    for (auto ia{0}; ia < alphaNumber_; ia++) {
            delete histsAmpByAlpha_[ia];
            histsAmpByAlpha_[ia] = nullptr;
            delete histsTimeCorrectedByAlpha_[ia];
            histsTimeCorrectedByAlpha_[ia] = nullptr;
            delete histsEnergyByAlpha_[ia];
            histsEnergyByAlpha_[ia] = nullptr;
    }
    for (auto ig{0}; ig < gammaNumber_; ig++) {
            delete histsAmpByGamma_[ig];
            histsAmpByGamma_[ig] = nullptr;
            delete histsAmpByGammaRc_[ig];
            histsAmpByGammaRc_[ig] = nullptr;

            delete histsEnergyByGamma_[ig];
            histsEnergyByGamma_[ig] = nullptr;
    }
}

void HistogramManager::printToPsFile(const std::string &fileName,
                             std::vector<std::shared_ptr<TH1> > &hists) const
{
    const std::string psName{(outputDirectory_.has_value() ? (outputDirectory_.value() + "/") : " ") + fileName + ".ps"};
    gErrorIgnoreLevel = 3'000;
    std::unique_ptr<TCanvas> c{new TCanvas("c", "c", 1024, 960)};
    c.get()->Print((psName + '[').c_str());
    for (size_t ig{0}; ig < hists.size(); ++ig)
    {
        hists.at(ig).get()->Draw();
        auto listOfFunctions{hists.at(ig).get()->GetListOfFunctions()};
        for (auto *item : *listOfFunctions)
        {
            item->Draw("SAME");
        }
        c.get()->Print(psName.c_str());
    }
    c.get()->Print((psName + ']').c_str());
    gErrorIgnoreLevel = 0;
}

void HistogramManager::printToPsFile(const std::string &fileName,
                             std::vector<std::vector<std::shared_ptr<TH1> > > &hists) const
{
    const std::string psName{(outputDirectory_.has_value() ? (outputDirectory_.value() + "/") : " ") + fileName + ".ps"};
    gErrorIgnoreLevel = 3'000;
    std::unique_ptr<TCanvas> c{new TCanvas("c", "c", 1024, 960)};
    c.get()->Print((psName + '[').c_str());
    for (size_t ig{0}; ig < hists.size(); ++ig)
    {
        auto cd{static_cast<int>(std::ceil(std::sqrt(hists.at(ig).size())))};
        c.get()->Divide(cd, cd);
        for (size_t ia{0}; ia <  hists.at(ig).size(); ++ia)
        {
            c.get()->cd(static_cast<int>(ia) + 1);
            hists.at(ig).at(ia).get()->Draw();
            auto listOfFunctions{hists.at(ig).at(ia).get()->GetListOfFunctions()};
            for (auto *item : *listOfFunctions)
            {
                item->Draw("SAME");
            }
        }
        c.get()->Print(psName.c_str());
        c.get()->Clear();
    }
    c.get()->Print((psName + ']').c_str());
    c.get()->Delete();
    gErrorIgnoreLevel = 0;
}

//void HistogramManager::printToPsFile(const std::string &fileName,
//                             std::vector<std::vector<std::shared_ptr<TH1> > > &hists) const
//{
//    const std::string psName{(_outputDirectory.has_value() ? (_outputDirectory.value() + "/") : " ") + fileName + ".ps"};
//    gErrorIgnoreLevel = 3'000;
//    std::unique_ptr<TCanvas> c{new TCanvas("c", "c", 1024, 960)};
//    c.get()->Print((psName + '[').c_str());
//    for (size_t ig{0}; ig < hists.size(); ++ig)
//    {
//        auto cd{static_cast<int>(std::ceil(std::sqrt(hists.at(ig).size())))};
//        c.get()->Divide(cd, cd);
//        for (size_t ia{0}; ia <  hists.at(ig).size(); ++ia)
//        {
//            c.get()->cd(static_cast<int>(ia) + 1);
//            hists.at(ig).at(ia).get()->Draw();
//            auto listOfFunctions{hists.at(ig).at(ia).get()->GetListOfFunctions()};
//            for (auto *item : *listOfFunctions)
//            {
//                item->Draw("SAME");
//            }
//        }
//        c.get()->Print(psName.c_str());
//        c.get()->Clear();
//    }
//    c.get()->Print((psName + ']').c_str());
//    gErrorIgnoreLevel = 0;
//}

void HistogramManager::printToPsFile(const std::string &fileName, std::shared_ptr<TH1> hist) const
{
    const std::string psName{(outputDirectory_.has_value() ? (outputDirectory_.value() + "/") : " ") + fileName + ".ps"};
    gErrorIgnoreLevel = 3'000;
    std::unique_ptr<TCanvas> c{new TCanvas("c", "c", 1024, 960)};
    c.get()->Print((psName + '[').c_str());
    hist.get()->Draw();
    auto listOfFunctions{hist->GetListOfFunctions()};
    for (auto *item : *listOfFunctions)
    {
//        item->Draw("SAME");
    }
    c.get()->Print(psName.c_str());
    c.get()->Print((psName + ']').c_str());
    gErrorIgnoreLevel = 0;
}

void HistogramManager::saveToRootFile(const std::string &fileName, std::shared_ptr<TH1> hist) const
{
    const std::string rootName{(outputDirectory_.has_value() ? (outputDirectory_.value() + "/") : " ") + fileName + ".root"};
    std::unique_ptr<TFile> file{TFile::Open(rootName.c_str(), "RECREATE")};
    if (file.get()->IsOpen())
    {
        hist.get()->Write(hist->GetName(), TObject::kOverwrite);
    }
}

void HistogramManager::resetAll()
{
    for (auto ig{0}; ig < gammaNumber_; ig++) {
        for (auto ia{0}; ia < alphaNumber_; ia++) {
            histsTimeByGammaAlpha_[ig][ia]->Reset();
            histsAmpByGammaAlphaSg_[ig][ia]->Reset();
            histsAmpByGammaAlphaBg_[ig][ia]->Reset();
            histsAmpByGammaAlphaRc_[ig][ia]->Reset();
            histsEnergyByGammaAlphaSg_[ig][ia]->Reset();
            histsEnergyByGammaAlphaBg_[ig][ia]->Reset();
        }
    }
    for (auto ia{0}; ia < alphaNumber_; ia++) {
            histsAmpByAlpha_[ia]->Reset();
            histsTimeCorrectedByAlpha_[ia]->Reset();
            histsEnergyByAlpha_[ia]->Reset();
    }
    for (auto ig{0}; ig < gammaNumber_; ig++) {
            histsAmpByGamma_[ig]->Reset();
            histsAmpByGammaRc_[ig]->Reset();
            histsEnergyByGamma_[ig]->Reset();
    }
}

const std::vector<std::vector<TH1D *> > &HistogramManager::histsAmpByGammaAlpha() const
{
    return histsAmpByGammaAlphaSg_;
}

const std::vector<std::vector<TH1D *> > &HistogramManager::histsTimeByGammaAlpha() const
{
    return histsTimeByGammaAlpha_;
}

const std::vector<TH1D *> &HistogramManager::histsAmpByAlpha() const
{
    return histsAmpByAlpha_;
}

const std::vector<std::vector<TH1D *> > &HistogramManager::histsTimeCorrectedByGammaAlpha() const
{
    return histsTimeCorrectedByGammaAlpha_;
}

const std::vector<TH1D *> &HistogramManager::histsTimeCorrectedByAlpha() const
{
    return histsTimeCorrectedByAlpha_;
}

const std::vector<std::vector<TH1D *> > &HistogramManager::histsAmpByGammaAlphaSg() const
{
    return histsAmpByGammaAlphaSg_;
}

const std::vector<std::vector<TH1D *> > &HistogramManager::histsAmpByGammaAlphaBg() const
{
    return histsAmpByGammaAlphaBg_;
}

const std::vector<TH1D *> &HistogramManager::histsAmpByGamma() const
{
    return histsAmpByGamma_;
}

const std::vector<TH1D *> &HistogramManager::histsAmpByGammaRc() const
{
    return histsAmpByGammaRc_;
}

const std::vector<std::vector<TH1D *> > &HistogramManager::histsAmpByGammaAlphaRc() const
{
    return histsAmpByGammaAlphaRc_;
}

const std::vector<std::vector<TH1D *> > &HistogramManager::histsEnergyByGammaAlphaSg() const
{
    return histsEnergyByGammaAlphaSg_;
}

const std::vector<std::vector<TH1D *> > &HistogramManager::histsEnergyByGammaAlphaBg() const
{
    return histsEnergyByGammaAlphaBg_;
}

const std::vector<TH1D *> &HistogramManager::histsEnergyByGamma() const
{
    return histsEnergyByGamma_;
}

const std::vector<TH1D *> &HistogramManager::histsEnergyByAlpha() const
{
    return histsEnergyByAlpha_;
}
