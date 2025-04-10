#pragma once

#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"

#include <unordered_set>

namespace transport_catalogue {

class RequestHandler {
public:
    explicit RequestHandler(const TransportCatalogue& catalogue, renderer::MapRenderer& renderer);

    std::optional<BusInfo> GetBusInfo(const std::string_view& bus_name) const;

    const std::set<std::string>* GetBusesByStop(const std::string_view& stop_name) const;

    const svg::Document* RenderMap();

private:
    // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
    const TransportCatalogue& catalogue_;
    renderer::MapRenderer& renderer_;
};

}