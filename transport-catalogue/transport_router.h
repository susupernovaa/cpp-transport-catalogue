#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <variant>

namespace transport_catalogue {

class TransportRouteProcessor {
private:
    using Graph = graph::DirectedWeightedGraph<double>;
    using Edge = graph::Edge<double>;
    using EdgeId = graph::EdgeId;
    using VertexId = graph::VertexId;
    using VertexPair = std::pair<VertexId, VertexId>;

    static constexpr double meters_in_km = 1000.0;
    static constexpr double minutes_in_hour = 60.0;

    struct BusRouteInfo {
        std::string busname;
        int span_count = 0;
    };

public:
    struct RoutingSettings {
        int bus_wait_time = 0;
        int bus_velocity = 0;
    };

    TransportRouteProcessor(RoutingSettings settings, const TransportCatalogue& transport_catalogue);

    std::optional<RouteInfo> GetRoute(std::string_view from, std::string_view to) const;

private:
    std::unordered_map<VertexId, std::string> wait_vertex_id_to_stop_;
    std::unordered_map<std::string_view, VertexId> stop_to_wait_vertex_id_;
    std::unordered_map<std::string_view, VertexId> stop_to_bus_vertex_id_;
    std::unordered_map<EdgeId, BusRouteInfo> edge_id_to_busroute_info_;
    RoutingSettings settings_;
    const TransportCatalogue& catalogue_;
    Graph graph_;
    graph::Router<double> router_;

    Graph BuildGraph();

    void AddWaitEdges(Graph& result_graph);

    void AddBusRouteEdges(const Bus& bus, Graph& result_graph, bool adjacent_only);

    void AddBusEdges(Graph& result_graph);
    
    RouteInfo BuildRouteInfo(const graph::Router<double>::RouteInfo& route) const;
};

} // namespace transport_catalogue