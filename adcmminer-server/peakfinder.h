#ifndef PEAKFINDER_H
#define PEAKFINDER_H

#include <TMath.h>
#include <TH1.h>
#include <TF1.h>

#include "energypeak.h"

class PeakFinder
{
public:
    PeakFinder();
    void process(TH1D *hist, TH1D * histRc);
    void processRaw(TH1D *hist);
    const std::vector<EnergyPeak> &energyPeaks() const;

    const EnergyPeak &energyPeak() const;

private:
    double calib_;
    double offset_;

    double getE(double ch) { return ch * calib_ + offset_; }
    double getdE(double dCh) { return dCh * calib_; }
    double getCh(double e) { return (e - offset_) / calib_; }
    double getdCh(double de) { return de / calib_; }

    double getFerrum847PosApprox(TH1 *h, double r = 0.12);
    double getFerrum847Pos(TH1 *h);
    double getFerrum1238Pos(TH1 *h);
    double getHydrogenPos(TH1 *h);
    double getCarbonPos(TH1 *h, double appPos);
    double getOxygenPos(TH1 *h, double appPos);
    double getFerrum7631Pos(TH1 *h, double appPos);

    std::vector<EnergyPeak>  energyPeaks_;
    EnergyPeak energyPeak_{EnergyPeak(EnergyPeak::Id::FE847, 0.0)};

};

#endif // PEAKFINDER_H
