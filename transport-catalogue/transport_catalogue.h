#pragma once
#include "geo.h"

#include <deque>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace transport_catalogue::database {

using namespace transport_catalogue::geo;

struct Stop {
	std::string name;
	Coordinates coords;
};

struct Bus {
	std::string name;
	std::vector<const Stop*> stops;
	bool is_circular;
};

struct BusInfo {
	std::string name;
	size_t stops_count;
	size_t unique_stops_count;
	int route_length;
	double curvature;
};

struct StopPairHasher {
	size_t operator()(const std::pair<const Stop*, const Stop*>& pair) const {
		return std::hash<const void*>()(pair.first) ^ std::hash<const void*>()(pair.second);
	}
};

class TransportCatalogue {
public:
	void AddStop(const std::string& name, const Coordinates& coords);
	void AddRoute(const std::string& name, const std::vector<std::string_view>& stops_names, bool is_circular);

	std::optional<BusInfo> GetRouteInfo(const std::string_view& name) const;
	std::optional<const std::set<std::string>*> GetBusesForStop(const std::string_view& name) const;
	std::optional<const Stop*> GetStopInfo(const std::string_view& name) const;
	void SetDistance(const Stop* from, const Stop* to, int distance);
	int GetDistance(const Stop* from, const Stop* to) const;

private:
	std::deque<Stop> stops_;
	std::deque<Bus> buses_;
	std::unordered_map<std::string_view, const Stop*> stops_by_name_;
	std::unordered_map<std::string_view, const Bus*> buses_by_name_;
	std::unordered_map<const Stop*, std::set<std::string>> stops_to_buses_;
	static const std::set<std::string> empty_buses_;
	std::unordered_map<std::pair<const Stop*, const Stop*>, int, StopPairHasher> distances_;
	
	int ComputeRouteDistance(const std::vector<const Stop*>& stops, size_t size) const;
};

}  // namespace transport_catalogue::database
