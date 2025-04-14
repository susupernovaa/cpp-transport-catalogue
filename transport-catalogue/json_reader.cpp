#include "json_reader.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <iterator>
#include <sstream>

namespace transport_catalogue {

namespace json_processing {

void JsonReader::ParseInput(std::istream& input) {
    using namespace json;
    using namespace std::literals;
    Node requests = Load(input).GetRoot();
    for (const auto& [key, value] : requests.AsMap()) {
        if (key == "base_requests"s) {
            ParseBaseRequests(value);
        } else if (key == "stat_requests"s) {
            ParseStatRequests(value);
        } else if (key == "render_settings"s) {
            ParseRenderSettings(value);
        }
    }
}

void JsonReader::FillBase(TransportCatalogue& catalogue) const {
    if (!base_stop_requests_.empty()) {
        for (const auto& stop : base_stop_requests_) {
            catalogue.AddStop(Stop{stop.name, 
                geo::Coordinates{stop.latitude, stop.longitude}});
        }
        for (const auto& stop : base_stop_requests_) {
            if (stop.road_distances) {
                catalogue.AddMapOfDistances(stop.name, stop.road_distances.value());
            }
        }
    }
    if (!base_bus_requests_.empty()) {
        for (const auto& bus : base_bus_requests_) {
            catalogue.AddBus(Bus{bus.name, 
                MakeVectorOfStops(bus.stops, bus.is_roundtrip, catalogue), bus.is_roundtrip});
        }
    }
}

const std::vector<StatRequest>& JsonReader::GetStatRequests() const {
    return stat_requests_;
}

const renderer::RenderSettings& JsonReader::GetRenderSettings() const {
    return render_settings_;
}

DistanceInfo JsonReader::ParseDistances(const json::Node& data) {
    DistanceInfo distances;
    for (const auto& [key, value] : data.AsMap()) {
        distances.insert({key, value.AsInt()});
    }
    return distances;
}

void JsonReader::ParseStopRequest(const json::Node& data) {
    using namespace std::literals;
    BaseStopRequest request;
    for (const auto& [key, value] : data.AsMap()) {
        if (key == "name"s) {
            request.name = value.AsString();
        } else if (key == "latitude"s) {
            request.latitude = value.AsDouble();
        } else if (key == "longitude"s) {
            request.longitude = value.AsDouble();
        } else if (key == "road_distances"s) {
            if (value == nullptr) {
                request.road_distances = std::nullopt;
            } else {
                request.road_distances = std::move(ParseDistances(value));
            }
        }
    }
    base_stop_requests_.push_back(std::move(request));
}

std::vector<std::string> JsonReader::ParseStops(const json::Node& data) {
    std::vector<std::string> stops;
    for (const auto& elem : data.AsArray()) {
        stops.push_back(elem.AsString());
    }
    return stops;
}

void JsonReader::ParseBusRequest(const json::Node& data) {
    using namespace std::literals;
    BaseBusRequest request;
    for (const auto& [key, value] : data.AsMap()) {
        if (key == "name"s) {
            request.name = value.AsString();
        } else if (key == "stops"s) {
            request.stops = std::move(ParseStops(value));
        } else if (key == "is_roundtrip"s) {
            request.is_roundtrip = value.AsBool();
        }
    }
    base_bus_requests_.push_back(std::move(request));
}

void JsonReader::ParseBaseRequest(const json::Node& request) {
    using namespace std::literals;
    for (const auto& [key, value] : request.AsMap()) {
        if (key == "type"s) {
            if (value.AsString() == "Stop"s) {
                ParseStopRequest(request); 
            } else if (value.AsString() == "Bus"s) {
                ParseBusRequest(request); 
            }
        }
    }
}

void JsonReader::ParseBaseRequests(const json::Node& base_requests) {
    for (const auto& request : base_requests.AsArray()) {
        ParseBaseRequest(request);
    } 
}

void JsonReader::ParseStatRequest(const json::Node& request_data) {
    using namespace std::literals;
    StatRequest request;
    for (const auto& [key, value] : request_data.AsMap()) {
        if (key == "id"s) {
            request.id = value.AsInt();
        } else if (key == "type"s) {
            request.type = value.AsString();
        } else if (key == "name"s) {
            request.name = value.AsString();
        }
    }
    stat_requests_.push_back(std::move(request));
}

void JsonReader::ParseStatRequests(const json::Node& stat_requests) {
    for (const auto& request : stat_requests.AsArray()) {
        ParseStatRequest(request);
    } 
}

std::vector<const Stop*> JsonReader::MakeVectorOfStops(const std::vector<std::string>& stops, 
        bool is_roundtrip, const TransportCatalogue& catalogue) {
    std::vector<const Stop*> busroute;
    for (const auto& stop : stops) {
        busroute.push_back(catalogue.GetStop(stop));
    }
    if (!is_roundtrip) {
        size_t size = busroute.size();
        for (size_t i = 0; i < size - 1; ++i) {
            busroute.push_back(busroute[size - (i + 1) - 1]);
        }
    }
    return busroute;
}

svg::Color GetColor(const json::Node& color) {
    svg::Color result;
    if (color.IsArray()) {
        if (color.AsArray().size() == 4) {
            auto array = color.AsArray();
            svg::Rgba rgba{static_cast<uint8_t>(array.at(0).AsInt()), 
                static_cast<uint8_t>(array.at(1).AsInt()), 
                static_cast<uint8_t>(array.at(2).AsInt()), 
                array.at(3).AsDouble()};
            result = rgba;
        } else if (color.AsArray().size() == 3) {
            auto array = color.AsArray();
            svg::Rgb rgb{static_cast<uint8_t>(array.at(0).AsInt()), 
                static_cast<uint8_t>(array.at(1).AsInt()), 
                static_cast<uint8_t>(array.at(2).AsInt())};
            result = rgb;
        }
    } else if (color.IsString()) {
        result = color.AsString();
    }
    return result;
}

void JsonReader::ParseRenderSettings(const json::Node& render_settings) {
    using namespace renderer;
    using namespace std::literals;
    for (const auto& [key, value] : render_settings.AsMap()) {
        if (key == "width"s) {
            render_settings_.width = value.AsDouble();
        } else if (key == "height"s) {
            render_settings_.height = value.AsDouble();
        } else if (key == "padding"s) {
            render_settings_.padding = value.AsDouble();
        } else if (key == "line_width"s) {
            render_settings_.line_width = value.AsDouble();
        } else if (key == "stop_radius"s) {
            render_settings_.stop_radius = value.AsDouble();
        } else if (key == "bus_label_font_size"s) {
            render_settings_.bus_label_font_size = value.AsInt();
        } else if (key == "bus_label_offset"s) {
            render_settings_.bus_label_offset.x = value.AsArray().at(0).AsDouble();
            render_settings_.bus_label_offset.y = value.AsArray().at(1).AsDouble();
        } else if (key == "stop_label_font_size"s) {
            render_settings_.stop_label_font_size = value.AsInt();
        } else if (key == "stop_label_offset"s) {
            render_settings_.stop_label_offset.x = value.AsArray().at(0).AsDouble();
            render_settings_.stop_label_offset.y = value.AsArray().at(1).AsDouble();
        } else if (key == "underlayer_color"s) {
            render_settings_.underlayer_color = GetColor(value);
        } else if (key == "underlayer_width"s) {
            render_settings_.underlayer_width = value.AsDouble();
        } else if (key == "color_palette"s) {
            for (const auto& elem : value.AsArray()) {
                render_settings_.color_palette.push_back(GetColor(elem));
            }
        }
    }
}

JsonPrinter::JsonPrinter(RequestHandler& request_handler, 
    const std::vector<StatRequest>& stat_requests) 
    : request_handler_(&request_handler), stats_(MakeStats(stat_requests)) {
}

void JsonPrinter::PrintStats(std::ostream& output) {
    json::Print(MakeJson(), output);
}

Stat JsonPrinter::ProcessStopRequest(const StatRequest& request) {
    StopData data;
    const auto* buses = request_handler_->GetBusesByStop(request.name);
    if (buses == nullptr) {
        data = std::nullopt;
    } else if (buses->empty()) {
        data = std::vector<std::string>();
    } else {
        data = std::vector<std::string>();
        for (const auto& bus : *buses) {
            data.value().push_back(bus);
        }
    }
    return Stat{request.id, data};
}

Stat JsonPrinter::ProcessBusRequest(const StatRequest& request) {
    BusData data;
    auto bus_info = request_handler_->GetBusInfo(request.name);
    if (!bus_info) {
        data = std::nullopt;
    } else {
        data = BusInfo{};
        data.value().stops = bus_info.value().stops;
        data.value().unique_stops = bus_info.value().unique_stops;
        data.value().route_length = bus_info.value().route_length;
        data.value().curvature = bus_info.value().curvature;
    }
    return Stat{request.id, data};
}

Stat JsonPrinter::ProcessMapRequest(const StatRequest& request) {
    const auto* data = request_handler_->RenderMap();
    return Stat{request.id, data};
}

std::vector<Stat> JsonPrinter::MakeStats(const std::vector<StatRequest>& stat_requests) {
    using namespace std::literals;
    std::vector<Stat> result;
    for (const auto& request : stat_requests) {
        if (request.type == "Stop"s) {
            result.push_back(ProcessStopRequest(request));
        } else if (request.type == "Bus"s) {
            result.push_back(ProcessBusRequest(request));
        } else if (request.type == "Map"s) {
            result.push_back(ProcessMapRequest(request));
        }
    }
    return result;
}

json::Document JsonPrinter::MakeJson() {
    using namespace json;
    using namespace std::literals;
    Array result;
    result.reserve(stats_.size());
    for (const auto& stat : stats_) {
        Dict stat_dict;
        stat_dict.insert({"request_id"s, stat.request_id});
        if (std::holds_alternative<StopData>(stat.data)) {
            StopData data = std::get<StopData>(stat.data);
            if (!data) {
                stat_dict.insert({"error_message"s, "not found"s});
            } else {
                Array buses;
                for (const auto& bus : data.value()) {
                    buses.push_back(bus);
                }
                stat_dict.insert({"buses"s, buses});
            }
        } else if (std::holds_alternative<BusData>(stat.data)) {
            BusData data = std::get<BusData>(stat.data);
            if (!data) {
                stat_dict.insert({"error_message"s, "not found"s});
            } else {
                stat_dict.insert({"stop_count"s, int(data.value().stops)});
                stat_dict.insert({"unique_stop_count"s, int(data.value().unique_stops)});
                stat_dict.insert({"route_length"s, data.value().route_length});
                stat_dict.insert({"curvature"s, data.value().curvature});
            }
        } else if (std::holds_alternative<const svg::Document*>(stat.data)) {
            const auto* data = std::get<const svg::Document*>(stat.data);
            std::ostringstream s;
            data->Render(s);
            stat_dict.insert({"map"s, s.str()});
        } 
        result.push_back(stat_dict);
    }
    return Document(result);
}

} // namespace json_processing

} // namespace transport_catalogue