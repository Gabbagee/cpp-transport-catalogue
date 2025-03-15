#include "transport_catalogue.h"

#include <unordered_set>

using namespace std;

namespace transport_catalogue::database {

void TransportCatalogue::AddStop(const string& name, const Coordinates& coords) {
    stops_.emplace_back(Stop{name, coords});
    stops_by_name_[name] = &stops_.back();
}

void TransportCatalogue::AddRoute(const string& name, const vector<string_view>& stops_names, bool is_circular) {
    Bus route{name, {}, is_circular};

    for (const auto& stop_name : stops_names) {
        if (stops_by_name_.count(stop_name) > 0) {
            const Stop* stop = stops_by_name_.at(stop_name);
            route.stops.push_back(stop);
            stops_to_buses_[stop].insert(name);
        }
    }

    buses_.push_back(move(route));
    buses_by_name_[name] = &buses_.back();
}

optional<Bus> TransportCatalogue::GetRouteInfo(const string& name) const {
    if (buses_by_name_.count(name) == 0) {
        return nullopt;
    }
    return *buses_by_name_.at(name);
}

optional<set<string>> TransportCatalogue::GetBusesForStop(const string& name) const {
    if (stops_by_name_.count(name) == 0) {
        return nullopt;
    }
    
    const Stop* stop = stops_by_name_.at(name);

    if (stops_to_buses_.count(stop) == 0) {
        return set<string>{};
    }

    return stops_to_buses_.at(stop);
}

} // namespace transport_catalogue::database