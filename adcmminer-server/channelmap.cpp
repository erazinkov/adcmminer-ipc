#include "channelmap.h"

ChannelMap::ChannelMap(std::vector<std::pair<u_int8_t, u_int8_t>> map)
{
    map_ = map;
}

ChannelMap ChannelMap::mapNAP()
{
    std::vector<std::pair<u_int8_t, u_int8_t>> map
    {
        {GAMMA, 0},
        {GAMMA, 1},
        {GAMMA, 2},
        {GAMMA, 3},
        {GAMMA, 4},
        {GAMMA, 5},
        {UNKNOWN, 111},
        {ALPHA, 0},
        {ALPHA, 1},
        {ALPHA, 2},
        {ALPHA, 3},
        {ALPHA, 4},
        {ALPHA, 5},
        {ALPHA, 6},
        {ALPHA, 7},
        {ALPHA, 8},
    };
    return ChannelMap(map);
}

ChannelMap ChannelMap::mapSTD()
{
    std::vector<std::pair<u_int8_t, u_int8_t>> map
    {
        {GAMMA, 0},
        {GAMMA, 1},
        {GAMMA, 2},
        {GAMMA, 3},
        {GAMMA, 4},
        {GAMMA, 5},
        {GAMMA, 6},
        {GAMMA, 7},
        {GAMMA, 8},
        {GAMMA, 9},
        {GAMMA, 10},
        {GAMMA, 11},
        {GAMMA, 12},
        {GAMMA, 13},
        {UNKNOWN, 111},
        {UNKNOWN, 111},
        {ALPHA, 0},
        {ALPHA, 1},
        {ALPHA, 2},
        {ALPHA, 3},
        {ALPHA, 4},
        {ALPHA, 5},
        {ALPHA, 6},
        {ALPHA, 7},
        {ALPHA, 8},
        {UNKNOWN, 111},
        {UNKNOWN, 111},
        {UNKNOWN, 111},
        {UNKNOWN, 111},
        {UNKNOWN, 111},
        {UNKNOWN, 111},
        {UNKNOWN, 111},
    };
    return ChannelMap(map);
}

unsigned long ChannelMap::numberOfChannelsAlpha() const
{
    return numberOfChannels(ALPHA);
}

unsigned long ChannelMap::numberOfChannelsGamma() const
{
    return numberOfChannels(GAMMA);
}

u_int8_t ChannelMap::numberByChannel(unsigned long ch) const
{
    return map_.at(ch).second;
}

u_int8_t ChannelMap::typeByChannel(unsigned long ch) const
{
    return map_.at(ch).first;
}

unsigned long ChannelMap::numberOfChannels(EChannelType type) const
{
    unsigned long number{};
    auto it = map_.begin();
    while ( (it = std::find_if(it, map_.end(), [&type](std::pair<u_int8_t, int> mapItem){return mapItem.first == type;}) ) != map_.end())
    {
        ++number;
        ++it;
    }
    return number;
}

bool ChannelMap::isCorrect(std::vector<u_int8_t> &map) const {
    if (map.size() != map_.size())
    {
        return false;
    }

    for (size_t i{0}; i < map_.size(); ++i)
    {
        if (map_[i].first == UNKNOWN)
        {
            continue;
        }
        if ( (map.at(i) & map_.at(i).first) != map_.at(i).first )
        {
            return false;
        }
    }
    return true;
}

const std::vector<std::pair<u_int8_t, u_int8_t> > &ChannelMap::map() const
{
    return map_;
}

