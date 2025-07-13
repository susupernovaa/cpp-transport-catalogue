#pragma once

#include "geo.h"

#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace transport_catalogue {

struct Stop {
    std::string stopname;
    geo::Coordinates coordinates;
};

struct Bus {
    std::string busname;
    std::vector<const Stop*> busroute;
    bool is_roundtrip;
};

struct BusInfo {
    bool operator==(const BusInfo& other) const {
        return stops == other.stops && unique_stops == other.unique_stops && 
        route_length - other.route_length < 1e-100 && curvature - other.curvature < 1e-100;
    }
    bool operator!=(const BusInfo& other) const {
        return !(*this == other);
    }
    size_t stops;
    size_t unique_stops;
    double route_length;
    double curvature;
};

struct StopDistHasher {
	size_t operator()(const std::pair<const Stop*, const Stop*>& stops) const {
		return std::hash<const void*>{}(stops.first) + 11 * std::hash<const void*>{}(stops.second);
	}
};

using DistancesContainer = std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopDistHasher>;

struct WaitItem {
    std::string stopname;
    double time = 0.0;
};

struct BusItem {
    std::string busname;
    int span_count = 0;
    double time = 0.0;
};

using Item = std::variant<WaitItem, BusItem>;

struct RouteInfo {
    double total_time = 0.0;
    std::vector<Item> items;
};

} // namespace transport_catalogue