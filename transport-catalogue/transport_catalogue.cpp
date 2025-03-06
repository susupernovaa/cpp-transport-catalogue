#include "transport_catalogue.h"

#include <algorithm>
#include <utility>
#include <unordered_set>

namespace transport_catalogue {

void TransportCatalogue::AddStop(Stop stop) {
    stops_.push_back(std::move(stop));
    std::string_view key = std::string_view(stops_[stops_.size() - 1].stopname);
    const Stop* adr = &(stops_.back());
    stopname_to_stop[key] = adr;
}

void TransportCatalogue::AddBus(Bus bus) {
    buses_.push_back(std::move(bus));
    std::string_view key = std::string_view(buses_[buses_.size() - 1].busname);
    const Bus* adr = &(buses_.back());
    busname_to_bus[key] = adr;
    for (const Stop* stop : buses_.back().busroute) {
        stopname_to_buses[stop].insert(buses_.back().busname);
    }
}

const Stop* TransportCatalogue::GetStop(std::string_view stopname) const {
    auto it = std::find(stopname_to_stop.begin(), stopname_to_stop.end(), stopname);
    if (it != stopname_to_stop.end()) {
        return it->second;
    }
    return nullptr;
}

const Bus* TransportCatalogue::GetBus(std::string_view busname) const {
    auto it = std::find(busname_to_bus.begin(), busname_to_bus.end(), busname);
    if (it != busname_to_bus.end()) {
        return it->second;
    }
    return nullptr;
}

std::optional<BusInfo> TransportCatalogue::GetBusInfo(std::string_view busname) const {
    size_t stops = 0;
    size_t unique_stops = 0;
    double route_length = 0.;
    if (GetBus(busname) != nullptr) {
        return {};
    }
    const Bus* bus = busname_to_bus.at(busname);
    stops = bus->busroute.size();
    std::unordered_set<const Stop*> unique_stops_set;
    const Stop* prev = bus->busroute[0];
    for (const auto* stop : bus->busroute) {
        unique_stops_set.insert(stop);
        route_length += ComputeDistance(prev->coordinates, stop->coordinates);
        prev = stop;
    }
    unique_stops = unique_stops_set.size();
    return BusInfo{stops, unique_stops, route_length};
}

const std::set<std::string>* TransportCatalogue::GetStopInfo(std::string_view stopname) const {
    if (GetStop(stopname) == nullptr) {
        return nullptr;
    }
    auto stop = GetStop(stopname);
    if (stopname_to_buses.count(stop) == 0) {
        static std::set<std::string> empty_set;
        return &empty_set;
    }
    return &stopname_to_buses.at(stop);
}

}