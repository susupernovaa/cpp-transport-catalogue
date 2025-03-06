#include "stat_reader.h"

#include <iostream>

namespace transport_catalogue {

namespace text_processing {

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output) {
    using namespace std::literals;
    const auto space = request.find(' ');
    if (space == request.npos) {
        return;
    }
    auto request_type = request.substr(0, space);
    auto text = request.substr(space + 1);
    if (request_type == "Bus"sv) {
        auto data = transport_catalogue.GetBusInfo(text);
        if (!data) {
            output << request << ": not found"s << std::endl;
        } else {
            output << request << ": "s << data.value().stops << " stops on route, "s << 
            data.value().unique_stops << " unique stops, "s << 
            data.value().route_length << " route length "s << std::endl;
        }
    }
    if (request_type == "Stop"sv) {
        const auto* stop_info = transport_catalogue.GetStopInfo(text);
        if (stop_info == nullptr) {
            output << request << ": not found"s << std::endl;
        } else if ((*stop_info).empty()) {
            output << request << ": no buses"s << std::endl;
        } else {
            output << request << ": buses "s;
            for (const auto& info : *stop_info) {
                output << info << ' ';
            }
            output << std::endl;
        }
    }
}

}

}