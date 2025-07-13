#include "transport_router.h"

namespace transport_catalogue {

TransportRouteProcessor::TransportRouteProcessor(RoutingSettings settings, 
        const TransportCatalogue& transport_catalogue) 
    : settings_(settings), 
    catalogue_(transport_catalogue), 
    graph_(BuildGraph()), 
    router_(graph_) {
}

std::optional<RouteInfo> TransportRouteProcessor::GetRoute(std::string_view from, std::string_view to) const {
    VertexId from_vertex = stop_to_wait_vertex_id_.at(from);
    VertexId to_vertex = stop_to_wait_vertex_id_.at(to);

    auto route = router_.BuildRoute(from_vertex, to_vertex);

    if (!route) {
        return std::nullopt;
    }

    return BuildRouteInfo(route.value());
}

graph::DirectedWeightedGraph<double> TransportRouteProcessor::BuildGraph() {
    Graph result_graph(catalogue_.GetAllStops().size() * 2);

    AddWaitEdges(result_graph);
    AddBusEdges(result_graph);

    return result_graph;
}

void TransportRouteProcessor::AddWaitEdges(Graph& result_graph) {
    VertexId vertex_id = 0;

    for (const auto& stop : catalogue_.GetAllStops()) {
        auto [it, _] = wait_vertex_id_to_stop_.emplace(vertex_id, stop.stopname);
        std::string_view stopname = it->second;

        stop_to_wait_vertex_id_[stopname] = vertex_id;
        stop_to_bus_vertex_id_[stopname] = vertex_id + 1;

        Edge edge{vertex_id, vertex_id + 1, static_cast<double>(settings_.bus_wait_time)};
        result_graph.AddEdge(edge);

        vertex_id += 2;
    }
}

void TransportRouteProcessor::AddBusRouteEdges(const Bus& bus, Graph& result_graph, bool adjacent_only) {
    const auto& stops = bus.busroute;

    if (stops.size() < 2) {
        return;
    }

    for (size_t from_stop_id = 0; from_stop_id < stops.size(); ++from_stop_id) {
        for (size_t to_stop_id = from_stop_id + 1; to_stop_id < stops.size(); ++to_stop_id) {
            if (adjacent_only && to_stop_id != from_stop_id + 1) {
                continue;
            }

            double total_distance = 0.0;
            int span_count = 0;

            for (size_t i = from_stop_id; i < to_stop_id; ++i) {
                const Stop* current_stop = stops[i];
                const Stop* next_stop = stops[i + 1];

                total_distance += catalogue_.GetDistance(current_stop->stopname, next_stop->stopname);
                span_count++;
            }

            const Stop* from_stop = stops[from_stop_id];
            const Stop* to_stop = stops[to_stop_id];

            VertexId from_vertex_id = stop_to_bus_vertex_id_.at(from_stop->stopname);
            VertexId to_vertex_id = stop_to_wait_vertex_id_.at(to_stop->stopname);

            double travel_time = total_distance / (settings_.bus_velocity * meters_in_km / minutes_in_hour);

            Edge edge{from_vertex_id, to_vertex_id, travel_time};
            edge_id_to_busroute_info_[result_graph.AddEdge(edge)] = {bus.busname, span_count};
        }
    }
}

void TransportRouteProcessor::AddBusEdges(Graph& result_graph) {
    for (const auto& bus : catalogue_.GetAllBuses()) {
        AddBusRouteEdges(bus, result_graph, true);
        AddBusRouteEdges(bus, result_graph, false);
    }
}

RouteInfo TransportRouteProcessor::BuildRouteInfo(const graph::Router<double>::RouteInfo& route) const {
    RouteInfo result;
    result.total_time = route.weight;

    for (EdgeId edge_id : route.edges) {
        const auto& edge = graph_.GetEdge(edge_id);

        if (edge.from % 2 == 0 && edge.to % 2 != 0) {  // wait edge
            result.items.emplace_back(WaitItem{std::string(wait_vertex_id_to_stop_.at(edge.from)), 
                edge.weight});
        } else { // bus edge
            result.items.emplace_back(BusItem{edge_id_to_busroute_info_.at(edge_id).busname, 
                edge_id_to_busroute_info_.at(edge_id).span_count, edge.weight});
        }
    }

    return result;
}

} // namespace transport_catalogue