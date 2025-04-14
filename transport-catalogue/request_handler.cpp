#include "request_handler.h"

namespace transport_catalogue {

RequestHandler::RequestHandler(const TransportCatalogue& catalogue, 
        renderer::MapRenderer& renderer) 
    : catalogue_(catalogue), renderer_(renderer) {
}

// Возвращает информацию о маршруте (запрос Bus)
std::optional<BusInfo> RequestHandler::GetBusInfo(const std::string_view& bus_name) const {
    return catalogue_.GetBusInfo(bus_name);
}

// Возвращает маршруты, проходящие через
const std::set<std::string>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
    return catalogue_.GetStopInfo(stop_name);
}

const svg::Document* RequestHandler::RenderMap() {
    return renderer_.RenderRoutes(catalogue_.GetAllBuses(), catalogue_.GetAllStopsInRoutes());
}

}