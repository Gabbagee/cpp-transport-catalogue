#pragma once

#include "geo.h"

#include <string>
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

}  // namespace transport_catalogue::database

