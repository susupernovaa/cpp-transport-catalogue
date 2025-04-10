#pragma once

#include <deque>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "domain.h"

namespace transport_catalogue {

class TransportCatalogue {
public:
	void AddStop(Stop stop);

	void AddBus(Bus bus);

	const Stop* GetStop(std::string_view stopname) const;

	const Bus* GetBus(std::string_view busname) const;

	std::optional<BusInfo> GetBusInfo(std::string_view busname) const;

	const std::set<std::string>* GetStopInfo(std::string_view stopname) const;

	void AddDistance(std::string_view stopname1, std::string_view stopname2, int distance);

	void AddMapOfDistances(std::string_view stopname, 
		std::unordered_map<std::string, int> stopnames_to_distances);

	int GetDistance(std::string_view stopname1, std::string_view stopname2) const;
	
	std::vector<geo::Coordinates> GetAllCoordinates() const;

	std::deque<Bus> GetAllBuses() const;

	std::deque<Stop> GetAllStopsInRoutes() const;

private:
	std::deque<Stop> stops_;
	std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
	std::deque<Bus> buses_;
	std::unordered_map<std::string_view, const Bus*> busname_to_bus_;
	std::unordered_map<const Stop*, std::set<std::string>> stopname_to_buses_;
	DistancesContainer stops_to_distances_;
};

}