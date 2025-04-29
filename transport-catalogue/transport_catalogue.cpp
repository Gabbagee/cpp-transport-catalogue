#include "transport_catalogue.h"

#include <iterator>

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

    if (!is_circular && !route.stops.empty()) {
        route.stops.insert(route.stops.end(), next(route.stops.rbegin()), route.stops.rend());
    }

    buses_.emplace_back(move(route));
    buses_by_name_[name] = &buses_.back();
}

int TransportCatalogue::ComputeRouteDistance(const vector<const Stop*>& stops, size_t size) const {
    int length = 0;

    for (size_t i = 0; i < size - 1; ++i) {
        const Stop* from = stops[i];
        const Stop* to = stops[i + 1];
        length += GetDistance(from, to);
    }

    return length;
}

optional<BusInfo> TransportCatalogue::GetRouteInfo(const string_view& name) const {
    auto it = buses_by_name_.find(name);
    if (it == buses_by_name_.end()) {
        return nullopt;
    }

    const Bus* route = it->second;
    const auto& stops = route->stops;
    size_t size = stops.size();
    unordered_set<string_view> unique_stops;
    double geo_length = 0.;
    int map_length = ComputeRouteDistance(stops, size);

    for (size_t i = 0; i < stops.size() - 1; ++i) {
        geo_length += ComputeDistance(stops[i]->coords, stops[i + 1]->coords);
        unique_stops.insert(stops[i]->name);
    }

    unique_stops.insert(stops.back()->name);
    double curvature = (geo_length > 0.) ? map_length / geo_length : 0.;

    return BusInfo{route->name, size, unique_stops.size(), map_length, curvature};
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

optional<const Stop*> TransportCatalogue::GetStopInfo(const string_view& name) const {
    auto it = stops_by_name_.find(name);
    if (it == stops_by_name_.end()) {
        return nullopt;
    }

    return it->second;
}

optional<set<const Bus*, BusNameComparator>> TransportCatalogue::GetAllBuses() const {
    set<const Bus*, BusNameComparator> all_buses;
    if (buses_by_name_.empty()) {
        return nullopt;
    }

    for (const auto& [_, bus_ptr] : buses_by_name_) {
        all_buses.insert(bus_ptr);
    }
    return all_buses;
}

optional<set<const Stop*, StopNameComparator>> TransportCatalogue::GetAllStops() const {
    set<const Stop*, StopNameComparator> all_stops;

    for (const auto& [_, bus_ptr] : buses_by_name_) {
        for (const auto* stop : bus_ptr->stops) {
            all_stops.insert(stop);
        }
    }

    if (all_stops.empty()) {
        return nullopt;
    }
    return all_stops;
}

void TransportCatalogue::SetDistance(const Stop* from, const Stop* to, int distance) {
    distances_[{from, to}] = distance;
}

int TransportCatalogue::GetDistance(const Stop* from, const Stop* to) const {
    auto it = distances_.find({from, to});
    if (it != distances_.end()) {
        return it->second;
    }
    
    return 0;
}

} // namespace transport_catalogue::database