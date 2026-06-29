#ifndef DECODER_H
#define DECODER_H

#include "adcm_df.h"
#include "channelmap.h"

#include <map>
#include <unordered_map>

struct PairHash {
    size_t operator()(const std::pair<uint8_t, uint8_t>& p) const {
        return (static_cast<size_t>(p.first) << 8) | p.second;
    }
};

class Decoder
{
public:
    Decoder();
    void process(const std::string &fileName);
    void process_o(const std::string &fileName);
    const dec_ch_t &channels() const;

    const std::map<std::pair<uint8_t, uint8_t>, std::vector<dec_ev_m_t> > &events() const;

    double time() const;

    const std::map<uint8_t, uint32_t> &counters() const;

    const std::unordered_map<std::pair<uint8_t, uint8_t>, std::vector<dec_ev_m_t>, PairHash> &events_o() const;

private:
    std::ifstream ifs_;
    std::map<std::pair<uint8_t, uint8_t>, std::vector<dec_ev_m_t>> events_;
    std::unordered_map<std::pair<uint8_t, uint8_t>,
                           std::vector<dec_ev_m_t>, PairHash> events_o_;
    std::map<uint8_t, uint32_t> counters_;
    double time_;

    dec_ch_t channels_;
};


#endif // DECODER_H
