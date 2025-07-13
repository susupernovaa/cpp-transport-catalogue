#include "request_handler.h"

namespace transport_catalogue {

RequestHandler::RequestHandler(const TransportCatalogue& catalogue, 
        renderer::MapRenderer& renderer, const TransportRouteProcessor& route_processor) 
    : catalogue_(catalogue), renderer_(renderer), route_processor_(route_processor) {
}

std::optional<BusInfo> RequestHandler::GetBusInfo(std::string_view bus_name) const {
    return catalogue_.GetBusInfo(bus_name);
}

const std::set<std::string>* RequestHandler::GetBusesByStop(std::string_view stop_name) const {
    return catalogue_.GetStopInfo(stop_name);
}

const svg::Document* RequestHandler::RenderMap() {
    return renderer_.RenderRoutes(catalogue_.GetAllBuses(), catalogue_.GetAllStopsInRoutes());
}

std::optional<RouteInfo> RequestHandler::GetRoute(std::string_view from, std::string_view to) {
    return route_processor_.GetRoute(from, to);
}

}