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
        details::BusInfo data = transport_catalogue.GetBusInfo(text);
        if (data == details::BusInfo{0, 0, 0.}) {
            output << request << ": not found"s << std::endl;
        } else {
            output << request << ": "s << data.stops << " stops on route, "s << data.unique_stops << 
            " unique stops, "s << data.route_length << " route length "s << std::endl;
        }
    }
    if (request_type == "Stop"sv) {
        auto stop_info = transport_catalogue.GetStopInfo(text);
        if (!stop_info) {
            output << request << ": not found"s << std::endl;
        } else if (stop_info.value().empty()) {
            output << request << ": no buses"s << std::endl;
        } else {
            output << request << ": buses "s;
            for (const auto& info : stop_info.value()) {
                output << info << ' ';
            }
            output << std::endl;
        }
    }
}

}

}