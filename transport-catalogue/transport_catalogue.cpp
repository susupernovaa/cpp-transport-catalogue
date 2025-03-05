#include "transport_catalogue.h"

#include <utility>
#include <unordered_set>

namespace transport_catalogue {

void TransportCatalogue::AddStop(const Stop& stop) {
    stops_.push_back(std::move(stop));
    std::string_view key = std::string_view(stops_[stops_.size() - 1].stopname);
    const Stop* adr = &(stops_[stops_.size() - 1]);
    stopname_to_stop[key] = adr;
}

void TransportCatalogue::AddBus(const Bus& bus) {
    buses_.push_back(std::move(bus));
    std::string_view key = std::string_view(buses_[buses_.size() - 1].busname);
    const Bus* adr = &(buses_[buses_.size() - 1]);
    busname_to_bus[key] = adr;
}

void TransportCatalogue::AddBusToStopIndex(const Stop* stop, std::string_view busname) {
    stopname_to_buses[stop].insert(std::move(std::string(busname)));
}

const Stop* TransportCatalogue::GetStop(std::string_view stopname) const {
    if (stopname_to_stop.count(stopname) != 0) {
        return stopname_to_stop.at(stopname);
    }
    return nullptr;
}

const Bus* TransportCatalogue::GetBus(std::string_view busname) const {
    if (busname_to_bus.count(busname) != 0) {
        return busname_to_bus.at(busname);
    }
    return nullptr;
}

details::BusInfo TransportCatalogue::GetBusInfo(std::string_view busname) const {
    size_t stops = 0;
    size_t unique_stops = 0;
    double route_length = 0.;
    if (busname_to_bus.count(busname) != 0) {
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
    }
    return details::BusInfo{stops, unique_stops, route_length};
}

std::optional<std::set<std::string>> TransportCatalogue::GetStopInfo(std::string_view stopname) const {
    if (GetStop(stopname) == nullptr) {
        return {};
    }
    if (stopname_to_buses.count(GetStop(stopname)) == 0) {
        std::set<std::string> empty_set;
        return empty_set;
    }
    return stopname_to_buses.at(GetStop(stopname));
}

}