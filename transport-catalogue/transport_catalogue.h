#pragma once

#include <deque>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
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

namespace details {

struct BusInfo {
	bool operator==(const BusInfo& other) const {
        return stops == other.stops && unique_stops == other.unique_stops && 
		route_length - other.route_length < 1e-6;
    }
	bool operator!=(const BusInfo& other) const {
		return !(*this == other);
	}
	size_t stops;
	size_t unique_stops;
	double route_length;
};

}

class TransportCatalogue {
public:
	void AddStop(const Stop& stop);

	void AddBus(const Bus& bus);

	void AddBusToStopIndex(const Stop* stop, std::string_view busname);

	const Stop* GetStop(std::string_view stopname) const;

	const Bus* GetBus(std::string_view busname) const;

	details::BusInfo GetBusInfo(std::string_view busname) const;

	std::optional<std::set<std::string>> GetStopInfo(std::string_view stopname) const;
	
private:
	std::deque<Stop> stops_;
	std::unordered_map<std::string_view, const Stop*> stopname_to_stop;
	std::deque<Bus> buses_;
	std::unordered_map<std::string_view, const Bus*> busname_to_bus;
	std::unordered_map<const Stop*, std::set<std::string>> stopname_to_buses;
};

}