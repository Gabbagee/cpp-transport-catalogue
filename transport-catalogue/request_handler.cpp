#include "request_handler.h"

#include <sstream>
#include <set>
#include <string>
#include <string_view>

using namespace std;

namespace transport_catalogue::requesting {

RequestHandler::RequestHandler(const TransportCatalogue& catalogue) 
    : catalogue_(catalogue) {}

const json::Node RequestHandler::HandleRouteRequest(const json::Dict& request) const {
    json::Dict result;
    result["request_id"s] = request.at("id"s).AsInt();
    const auto& name = request.at("name"s).AsString();
    auto route_info = catalogue_.GetRouteInfo(name);

    if (!route_info.has_value()) {
        result["error_message"s] = json::Node{"not found"s};
        return json::Node{result};
    }

    const BusInfo& info = route_info.value();

    result["curvature"] = info.curvature;
    result["route_length"] = info.route_length;
    result["stop_count"] = static_cast<int>(info.stops_count);
    result["unique_stop_count"] = static_cast<int>(info.unique_stops_count);

    return json::Node{result};
}

const json::Node RequestHandler::HandleStopRequest(const json::Dict& request) const {
    json::Dict result;
    result["request_id"s] = request.at("id"s).AsInt();
    const auto& name = request.at("name"s).AsString();
    auto buses_ptr = catalogue_.GetBusesForStop(name);

    if (!buses_ptr.has_value()) {
        result["error_message"s] = json::Node{"not found"s};
        return json::Node{result};
    }

    json::Array buses_list;
    const set<string>* buses = buses_ptr.value();

    for (const auto& bus : *buses) {
        buses_list.emplace_back(bus);
    }

    result["buses"s] = move(buses_list);

    return json::Node{result};
}

const json::Node RequestHandler::HandleMapRequest(const json::Dict& request, MapRenderer& renderer) const {
    json::Dict result;
    result["request_id"s] = request.at("id"s).AsInt();

    svg::Document svg_map = renderer.RenderMap();

    ostringstream strm;
    svg_map.Render(strm);
    result["map"s] = strm.str();

    return json::Node{result};
}

} // namespace transport_catalogue::requesting