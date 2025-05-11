#include "domain.h"
#include "transport_router.h"

using namespace std;

namespace transport_catalogue::routing {

constexpr double KPHtoMPM = 1000.0 / 60.0;

void Router::BuildGraph() {
    auto all_buses = catalogue_.GetAllBuses();
    if (!all_buses.has_value()) {
        return;
    }

    auto all_stops = catalogue_.GetAllStops();
    if (!all_stops.has_value()) {
        return;
    }

    const set<const Bus*, BusNameComparator>& buses = all_buses.value();
    const set<const Stop*, StopNameComparator>& stops = all_stops.value();
    graph::DirectedWeightedGraph<double> graph(stops.size() * 2);
    map<string, graph::VertexId> stop_ids;
    graph::VertexId vertex_id = 0;

    for (const auto* stop : stops) {
        stop_ids[stop->name] = vertex_id;
        graph.AddEdge({
                stop->name,
                0,
                vertex_id,
                ++vertex_id,
                static_cast<double>(settings_.bus_wait_time)
            });
        ++vertex_id;
    }
    stop_ids_ = move(stop_ids);

    BuildEdgesForBuses(buses, graph);

    graph_ = move(graph);
    router_ = make_unique<graph::Router<double>>(graph_);
}

void Router::BuildEdgesForBuses(const set<const Bus*, BusNameComparator>& buses, graph::DirectedWeightedGraph<double>& graph) const {
    for (const auto* bus : buses) {
        const auto& stops = bus->stops;
        size_t stops_count = stops.size();

        for (size_t i = 0; i < stops_count; ++i) {
            for (size_t j = i + 1; j < stops_count; ++j) {
                const Stop* stop_from = stops[i];
                const Stop* stop_to = stops[j];
                int dist_sum = 0;

                for (size_t k = i + 1; k <= j; ++k) {
                    dist_sum += catalogue_.GetDistance(stops[k - 1], stops[k]);
                }

                graph.AddEdge({ bus->name,
                                j - i,
                                stop_ids_.at(stop_from->name) + 1,
                                stop_ids_.at(stop_to->name),
                                static_cast<double>(dist_sum) / (settings_.bus_velocity * KPHtoMPM)
                });
            }
        }
    }
}

const optional<RouteData> Router::FindRoute(const string_view stop_from, const string_view stop_to) const {
    RouteData route_data;
    auto from_it = stop_ids_.find(string(stop_from));
    auto to_it = stop_ids_.find(string(stop_to));

    if (from_it == stop_ids_.end() || to_it == stop_ids_.end()) {
        return nullopt;
    }

    graph::VertexId from_id = from_it->second;
    graph::VertexId to_id = to_it->second;
    const auto& route_info = router_->BuildRoute(from_id, to_id);

    if (!route_info.has_value()) {
        return nullopt;
    }

    route_data.total_time = route_info->weight;
    for (const auto& edge_id : route_info->edges) {
        route_data.edges.push_back(&graph_.GetEdge(edge_id));
    }

    return route_data;
}

} // namespace transport_catalogue::routing