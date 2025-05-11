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

struct RouteData {
	double total_time;
	std::vector<const graph::Edge<double>*> edges;
};

struct RoutingSettings {
	int bus_wait_time;
	double bus_velocity;
};

class Router {
public:
	Router(const RoutingSettings& settings, const TransportCatalogue& catalogue)
    : settings_(settings), catalogue_(catalogue) {
		BuildGraph();
	}

	const std::optional<RouteData> FindRoute(const std::string_view stop_from, const std::string_view stop_to) const;

private:
	const RoutingSettings settings_;
	const TransportCatalogue& catalogue_;
	graph::DirectedWeightedGraph<double> graph_;
	std::unique_ptr<graph::Router<double>> router_;
	
	std::map<std::string, graph::VertexId> stop_ids_;
	void BuildGraph();
	void BuildEdgesForBuses(const std::set<const Bus*, BusNameComparator>& buses, graph::DirectedWeightedGraph<double>& graph) const;
};

} // namespace transport_catalogue::routing