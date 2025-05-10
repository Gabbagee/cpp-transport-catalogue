#pragma once

#include "router.h"
#include "transport_catalogue.h"

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace transport_catalogue::routing {

using namespace transport_catalogue::database;

struct RoutingSettings {
	int bus_wait_time;
	double bus_velocity;
};

class Router {
public:
	Router(const RoutingSettings& settings, const TransportCatalogue& catalogue)
    : settings_(settings), catalogue_(catalogue) {}

	void BuildGraph();
	const std::optional<graph::Router<double>::RouteInfo> FindRoute(const std::string_view stop_from, const std::string_view stop_to) const;
	const graph::DirectedWeightedGraph<double>& GetGraph() const;

private:
	const RoutingSettings settings_;
    const TransportCatalogue& catalogue_;
	graph::DirectedWeightedGraph<double> graph_;
	std::unique_ptr<graph::Router<double>> router_;
	
	std::map<std::string, graph::VertexId> stop_ids_;
	void BuildEdgesForBuses(const std::set<const Bus*, BusNameComparator>& buses, graph::DirectedWeightedGraph<double>& graph) const;
};

} // namespace transport_catalogue::routing