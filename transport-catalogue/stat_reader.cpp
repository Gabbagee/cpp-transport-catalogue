#include "geo.h"
#include "stat_reader.h"

#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_set>

using namespace std;

namespace transport_catalogue::retrieving {

using namespace transport_catalogue::geo;

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, string_view request,
                       ostream& output) {
    if (request.substr(0, 3) == "Bus"s) {
        string name(request.substr(4));
        auto route = transport_catalogue.GetRouteInfo(name);
        
        if (!route.has_value()) {
            output << "Bus "s << name << ": not found"s << endl;
            return;
        }

        const auto& stops = route->stops;
        unordered_set<string_view> unique_stops;
        double length = 0.;

        for (size_t i = 0; i < stops.size() - 1; ++i) {
            length += ComputeDistance(stops[i]->coords, stops[i + 1]->coords);
            unique_stops.insert(stops[i]->name);
        }

        unique_stops.insert(stops.back()->name);

        output << "Bus "s << name << ": "s << stops.size() << " stops on route, "s
            << unique_stops.size() << " unique stops, "s
            << fixed << setprecision(6) << length << " route length"s << endl;
    }
    
    if (request.substr(0, 4) == "Stop"s) {
        string name(request.substr(5));
        auto buses = transport_catalogue.GetBusesForStop(name);

        if (!buses.has_value()) {
            output << "Stop "s << name << ": not found"s << endl;
            return;
        } else if (buses->empty()) {
            output << "Stop "s << name << ": no buses"s << endl;
            return;
        }

        output << "Stop "s << name << ": buses"s;
        
        for (const auto& bus : *buses) {
            output << " "s << bus;
        }

        output << endl;
    }
}

} // namespace transport_catalogue::retrieving