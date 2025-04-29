#include "stat_reader.h"

#include <iomanip>
#include <iostream>
#include <string>
#include <set>

using namespace std;

namespace transport_catalogue::retrieving {

void ParseAndPrintStat(const TransportCatalogue& catalogue, string_view request, ostream& output) {
    if (request.substr(0, 3) == "Bus"s) {
        string_view name(request.substr(4));
        auto route_info = catalogue.GetRouteInfo(name);
        
        if (!route_info.has_value()) {
            output << "Bus "s << name << ": not found"s << endl;
            return;
        }

        const BusInfo& info = route_info.value();

        output << "Bus "s << info.name << ": "s 
            << info.stops_count << " stops on route, "s 
            << info.unique_stops_count << " unique stops, "s 
            << info.route_length << " route length, "s 
            << fixed << setprecision(5) << info.curvature << " curvature"s << endl;
    }
    
    if (request.substr(0, 4) == "Stop"s) {
        string_view name(request.substr(5));
        auto buses_ptr = catalogue.GetBusesForStop(name);

        if (!buses_ptr.has_value()) {
            output << "Stop "s << name << ": not found"s << endl;
            return;
        }
        
        const set<string>* buses = buses_ptr.value();

        if (buses->empty()) {
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

void ReadStatRequests(const TransportCatalogue& catalogue, istream& input, ostream& output) {
    int stat_request_count;
    input >> stat_request_count >> ws;
    
    for (int i = 0; i < stat_request_count; ++i) {
        string line;
        getline(input, line);
        ParseAndPrintStat(catalogue, line, output);
    }
}

} // namespace transport_catalogue::retrieving