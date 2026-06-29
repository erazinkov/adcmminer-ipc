#ifndef PIECEWISELINEARFUNCTION_H
#define PIECEWISELINEARFUNCTION_H

#include "energypeak.h"
#include <vector>

class PiecewiseLinearFunction
{
public:
    PiecewiseLinearFunction(const std::vector<EnergyPeak> &energyPeaks);

    double operator() (double *x, double *) {
       double arg{x[0]};
       double val{0.0};
       val = arg;
       for (size_t i{0}; i < par_.size(); ++i) {
           if (arg < par_.at(i).node) {
               val = par_.at(i).intercept + arg * par_.at(i).slope;
               return val;
           }
       }
       return val;
   }
    auto parNumber() const {
        return par_.size();
    }
private:
    struct Par
    {
        double slope;
        double intercept;
        double node;
        bool operator < (const Par& p) const
        {
            return node < p.node;
        }
    };


    std::vector<Par> par_;

    Par par(const EnergyPeak &prev, const EnergyPeak &next);
};

#endif // PIECEWISELINEARFUNCTION_H
