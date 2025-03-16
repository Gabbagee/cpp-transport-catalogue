#include "transport_catalogue.h"

#include <unordered_set>

using namespace std;

namespace transport_catalogue::database {

using namespace transport_catalogue::geo;

const set<string> TransportCatalogue::empty_buses_ = {};

void TransportCatalogue::AddStop(const string& name, const Coordinates& coords) {
    stops_.emplace_back(Stop{name, coords});
    stops_by_name_[name] = &stops_.back();
}

void TransportCatalogue::AddRoute(const string& name, const vector<string_view>& stops_names, bool is_circular) {
    Bus route{name, {}, is_circular};

    for (const auto& stop_name : stops_names) {
        auto it = stops_by_name_.find(stop_name);
        if (it != stops_by_name_.end()) {
            const Stop* stop = it->second;
            route.stops.push_back(stop);
            stops_to_buses_[stop].insert(name);
        }
    }

    buses_.push_back(move(route));
    buses_by_name_[name] = &buses_.back();
}

optional<BusInfo> TransportCatalogue::GetRouteInfo(const string_view& name) const {
    auto it = buses_by_name_.find(name);
    if (it == buses_by_name_.end()) {
        return nullopt;
    }

    const Bus* route = it->second;
    const auto& stops = route->stops;
    unordered_set<string_view> unique_stops;
    double length = 0.;

    for (size_t i = 0; i < stops.size() - 1; ++i) {
        length += ComputeDistance(stops[i]->coords, stops[i + 1]->coords);
        unique_stops.insert(stops[i]->name);
    }

    unique_stops.insert(stops.back()->name);

    return BusInfo{route->name, stops.size(), unique_stops.size(), length};
}

optional<const set<string>*> TransportCatalogue::GetBusesForStop(const string_view& name) const {
    auto it = stops_by_name_.find(name);
    if (it == stops_by_name_.end()) {
        return nullopt;
    }
    
    const Stop* stop = it->second;
    auto it2 = stops_to_buses_.find(stop);
    if (it2 == stops_to_buses_.end()) {
        return &empty_buses_;
    }

    return &it2->second;
}

} // namespace transport_catalogue::database