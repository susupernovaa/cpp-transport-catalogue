#pragma once

#include <deque>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "geo.h"

namespace transport_catalogue {

struct Stop {
	std::string stopname;
	Coordinates coordinates;
};

struct Bus {
	std::string busname;
	std::vector<const Stop*> busroute;
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

class TransportCatalogue {
public:
	void AddStop(Stop stop);

	void AddBus(Bus bus);

	const Stop* GetStop(std::string_view stopname) const;

	const Bus* GetBus(std::string_view busname) const;

	std::optional<BusInfo> GetBusInfo(std::string_view busname) const;

	const std::set<std::string>* GetStopInfo(std::string_view stopname) const;

	void AddDistances(std::string_view stopname, 
		std::unordered_map<std::string, int> stopnames_to_distances);

	int GetDistance(std::string_view stopname1, std::string_view stopname2) const;
	
private:
	std::deque<Stop> stops_;
	std::unordered_map<std::string_view, const Stop*> stopname_to_stop;
	std::deque<Bus> buses_;
	std::unordered_map<std::string_view, const Bus*> busname_to_bus;
	std::unordered_map<const Stop*, std::set<std::string>> stopname_to_buses;
	std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopDistHasher> stops_to_distances_;
};

}