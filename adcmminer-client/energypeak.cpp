#include "energypeak.h"

EnergyPeak::EnergyPeak(EnergyPeak::Id id, double channel) :
    id_(id), channel_(channel)
{
    switch (id_) {
    case EnergyPeak::Id::FE847:
        energy_ = 847.0;
        break;
    case EnergyPeak::Id::FE1238:
        energy_ = 1238.0;
        break;
    case EnergyPeak::Id::HYDROGEN:
        energy_ = 2238.0;
        break;
    case EnergyPeak::Id::CARBON:
        energy_ = 4438.0;
        break;
    case EnergyPeak::Id::OXYGEN:
        energy_ = 6129.0;
        break;
    case EnergyPeak::Id::FE7631:
        energy_ = 7631.0;
        break;
    }
}

EnergyPeak::Id EnergyPeak::id() const
{
    return id_;
}

double EnergyPeak::channel() const
{
    return channel_;
}

double EnergyPeak::energy() const
{
    return energy_;
}
