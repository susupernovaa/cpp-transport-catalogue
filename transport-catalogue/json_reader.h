#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

namespace transport_catalogue {

namespace json_processing {

using DistanceInfo = std::unordered_map<std::string, int>;

struct BaseStopRequest {
    std::string name;
    double latitude;
    double longitude;
    std::optional<DistanceInfo> road_distances;
};

struct BaseBusRequest {
    std::string name;
    std::vector<std::string> stops;
    bool is_roundtrip;
};

struct StatRequest {
    int id;
    std::string type;
    std::string name;
};

class JsonReader {
public:
    void ParseInput(std::istream& input);

    void FillBase(TransportCatalogue& catalogue) const;

    const std::vector<StatRequest>& GetStatRequests() const;
    
    const renderer::RenderSettings& GetRenderSettings() const;

private:
    std::vector<BaseStopRequest> base_stop_requests_;
    std::vector<BaseBusRequest> base_bus_requests_;
    std::vector<StatRequest> stat_requests_;
    renderer::RenderSettings render_settings_;

    static DistanceInfo ParseDistances(const json::Node& data);

    void ParseStopRequest(const json::Node& data);

    static std::vector<std::string> ParseStops(const json::Node& data);

    void ParseBusRequest(const json::Node& data);

    void ParseBaseRequest(const json::Node& request);

    void ParseBaseRequests(const json::Node& base_requests);

    void ParseStatRequest(const json::Node& request_data);

    void ParseStatRequests(const json::Node& stat_requests);

    static std::vector<const Stop*> MakeVectorOfStops(const std::vector<std::string>& stops, 
        bool is_roundtrip, const TransportCatalogue& catalogue);

    void ParseRenderSettings(const json::Node& render_settings);
};

using StopData = std::optional<std::vector<std::string>>;
using BusData = std::optional<BusInfo>;

struct Stat {
    int request_id;
    std::variant<StopData, BusData, const svg::Document*> data;
};

class JsonPrinter {
public:
    explicit JsonPrinter(RequestHandler& request_handler, 
        const std::vector<StatRequest>& stat_requests);

    void PrintStats(std::ostream& output);

private:
    RequestHandler* request_handler_;
    std::vector<Stat> stats_;

    Stat ProcessStopRequest(const StatRequest& request);

    Stat ProcessBusRequest(const StatRequest& request);

    Stat ProcessMapRequest(const StatRequest& request);

    std::vector<Stat> MakeStats(const std::vector<StatRequest>& stat_requests);

    json::Document MakeJson();
};

} // namespace json_processing

} // namespace transport_catalogue