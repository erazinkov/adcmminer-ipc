#ifndef TIMEPEAKSFINDER_H
#define TIMEPEAKSFINDER_H

#include <vector>

#include <TH1.h>

#include "channelmap.h"

class TimePeaksFinder
{
public:
    TimePeaksFinder(const ChannelMap &map);

    void calculatePeaksPos(std::vector<std::vector<TH1 *>> hists);
    void calculatePeaksPos(std::vector<std::vector<std::shared_ptr<TH1>> > &hists);
    const std::vector<std::vector<double> > &timePeaksPos() const;
    void writePeaksPosToFile(const std::string &fileName);
    void readPeaksPosFromFile(const std::string &fileName);

private:
    std::vector<std::vector<double>> _timePeaksPos;

    double calculatePeakPos(TH1 *hist);

};

#endif // TIMEPEAKSFINDER_H
