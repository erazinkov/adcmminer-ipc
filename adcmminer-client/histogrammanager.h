#ifndef HISTOGRAMMANAGER_H
#define HISTOGRAMMANAGER_H

#include <memory>
#include <optional>

#include <TH1.h>
#include <TH1D.h>
#include <TH2.h>

class HistogramManager
{
public:
    HistogramManager(const int &gammaNumber, const int &alphaNumber, std::optional<std::string> outputDirectory = std::nullopt);
    ~HistogramManager();

    void printToPsFile(const std::string &psName,
               std::vector<std::vector<std::shared_ptr<TH1>> > &hists) const;
    void printToPsFile(const std::string &fileName,
                                 std::vector<std::shared_ptr<TH1> > &hists) const;
    void printToPsFile(const std::string &psName,
               std::shared_ptr<TH1> hist) const;
    void saveToRootFile(const std::string &fileName,
                        std::shared_ptr<TH1> hist) const;
    void resetAll();

    const std::vector<std::vector<TH1D *> > &histsAmpByGammaAlpha() const;
    const std::vector<std::vector<TH1D *> > &histsTimeByGammaAlpha() const;

    const std::vector<TH1D *> &histsAmpByAlpha() const;

    const std::vector<std::vector<TH1D *> > &histsTimeCorrectedByGammaAlpha() const;

    const std::vector<TH1D *> &histsTimeCorrectedByAlpha() const;

    const std::vector<std::vector<TH1D *> > &histsAmpByGammaAlphaSg() const;

    const std::vector<std::vector<TH1D *> > &histsAmpByGammaAlphaBg() const;

    const std::vector<TH1D *> &histsAmpByGamma() const;

    const std::vector<TH1D *> &histsAmpByGammaRc() const;

    const std::vector<std::vector<TH1D *> > &histsAmpByGammaAlphaRc() const;

    const std::vector<std::vector<TH1D *> > &histsEnergyByGammaAlphaSg() const;

    const std::vector<std::vector<TH1D *> > &histsEnergyByGammaAlphaBg() const;

    const std::vector<TH1D *> &histsEnergyByGamma() const;

    const std::vector<TH1D *> &histsEnergyByAlpha() const;

private:
    const int gammaNumber_;
    const int alphaNumber_;

    static constexpr int BINS_TIME{800};
    static constexpr int BINS_CHANNEL{400};
    static constexpr int BINS_ENERGY{640};

    static constexpr double XLOW_TIME{-200.0};
    static constexpr double XLOW_CHANNEL{0.0};
    static constexpr double XLOW_ENERGY{0.0};

    static constexpr double XUP_TIME{200.0};
    static constexpr double XUP_CHANNEL{4.0e3};
    static constexpr double XUP_ENERGY{8.0e3};

    std::optional<std::string> outputDirectory_;
    std::vector<std::vector<TH1D *>> histsAmpByGammaAlphaSg_;
    std::vector<std::vector<TH1D *>> histsAmpByGammaAlphaBg_;
    std::vector<std::vector<TH1D *>> histsAmpByGammaAlphaRc_;
    std::vector<std::vector<TH1D *>> histsTimeByGammaAlpha_;


    std::vector<std::vector<TH1D *>> histsTimeCorrectedByGammaAlpha_;
    std::vector<TH1D *> histsTimeCorrectedByAlpha_;

    std::vector<TH1D *> histsAmpByAlpha_;

    std::vector<TH1D *> histsAmpByGamma_;
    std::vector<TH1D *> histsAmpByGammaRc_;


    std::vector<std::vector<TH1D *>> histsEnergyByGammaAlphaSg_;
    std::vector<std::vector<TH1D *>> histsEnergyByGammaAlphaBg_;
    std::vector<TH1D *> histsEnergyByGamma_;
    std::vector<TH1D *> histsEnergyByAlpha_;
};

#endif // HISTOGRAMMANAGER_H
