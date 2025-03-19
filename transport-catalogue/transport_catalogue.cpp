#include "transport_catalogue.h"

#include <iostream>
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
    auto it = stopname_to_stop.find(stopname);
    if (it != stopname_to_stop.end()) {
        return it->second;
    }
    return nullptr;
}

const Bus* TransportCatalogue::GetBus(std::string_view busname) const {
    auto it = busname_to_bus.find(busname);
    if (it != busname_to_bus.end()) {
        return it->second;
    }
    return nullptr;
}

std::optional<BusInfo> TransportCatalogue::GetBusInfo(std::string_view busname) const {
    if (GetBus(busname) == nullptr) {
        return {};
    }
    const Bus* bus = busname_to_bus.at(busname);
    size_t stops = bus->busroute.size();
    std::unordered_set<const Stop*> unique_stops_set;
    const Stop* prev = bus->busroute[0];
    double geo_l = 0.;
    double l = 0.;
    bool is_first = true;
    for (const auto* stop : bus->busroute) {
        unique_stops_set.insert(stop);
        geo_l += ComputeDistance(prev->coordinates, stop->coordinates);
        l += GetDistance(prev->stopname, stop->stopname);
        if (!is_first) {
            l += GetDistance(stop->stopname, stop->stopname);
        }
        prev = stop;
        is_first = false;
    }
    size_t unique_stops = unique_stops_set.size();
    return BusInfo{stops, unique_stops, l, l / geo_l};
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

void TransportCatalogue::AddDistances(std::string_view stopname1, 
    std::unordered_map<std::string, int> stopnames_to_distances) {
        const auto* stop1 = GetStop(stopname1);
        for (const auto& [stopname2, distance] : stopnames_to_distances) {
            const auto* stop2 = GetStop(stopname2);
            stops_to_distances_[{stop1, stop2}] = distance;
        }
}

int TransportCatalogue::GetDistance(std::string_view stopname1, std::string_view stopname2) const {
    const auto* stop1 = GetStop(stopname1);
    const auto* stop2 = GetStop(stopname2);
    auto it = stops_to_distances_.find({stop1, stop2});
    if (it == stops_to_distances_.end()) {
        it = stops_to_distances_.find({stop2, stop1});
    }
    return it != stops_to_distances_.end() ? it->second : 0;
}

}