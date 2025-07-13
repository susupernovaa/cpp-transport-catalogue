#pragma once

#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <unordered_set>

namespace transport_catalogue {

class RequestHandler {
public:
    explicit RequestHandler(const TransportCatalogue& catalogue, renderer::MapRenderer& renderer,
        const TransportRouteProcessor& route_processor);

    std::optional<BusInfo> GetBusInfo(std::string_view bus_name) const;

    const std::set<std::string>* GetBusesByStop(std::string_view stop_name) const;

    const svg::Document* RenderMap();

    std::optional<RouteInfo> GetRoute(std::string_view from, std::string_view to);

private:
    const TransportCatalogue& catalogue_;
    renderer::MapRenderer& renderer_;
    const TransportRouteProcessor& route_processor_;
};

}