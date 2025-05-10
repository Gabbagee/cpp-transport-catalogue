#include "domain.h"
#include "json_builder.h"
#include "request_handler.h"
#include "router.h"

#include <sstream>
#include <set>
#include <string>
#include <string_view>

using namespace std;

namespace transport_catalogue::requesting {

RequestHandler::RequestHandler(const TransportCatalogue& catalogue) 
    : catalogue_(catalogue) {}

const json::Node RequestHandler::HandleRouteRequest(const json::Dict& request) const {
    json::Node result;
    int id = request.at("id"s).AsInt();
    const auto& name = request.at("name"s).AsString();
    auto route_info = catalogue_.GetRouteInfo(name);

    if (!route_info.has_value()) {
        return result = json::Builder{}
                    .StartDict()
                        .Key("request_id").Value(id)
                        .Key("error_message").Value("not found")
                    .EndDict()
                .Build();
    }

    const BusInfo& info = route_info.value();

    result = json::Builder{}
                    .StartDict()
                        .Key("request_id").Value(id)
                        .Key("curvature").Value(info.curvature)
                        .Key("route_length").Value(static_cast<double>(info.route_length))
                        .Key("stop_count").Value(static_cast<int>(info.stops_count))
                        .Key("unique_stop_count").Value(static_cast<int>(info.unique_stops_count))
                    .EndDict()
                .Build();

    return result;
}

const json::Node RequestHandler::HandleStopRequest(const json::Dict& request) const {
    json::Node result;
    int id = request.at("id"s).AsInt();
    const auto& name = request.at("name"s).AsString();
    auto buses_ptr = catalogue_.GetBusesForStop(name);

    if (!buses_ptr.has_value()) {
        return result = json::Builder{}
                    .StartDict()
                        .Key("request_id").Value(id)
                        .Key("error_message").Value("not found")
                    .EndDict()
                .Build();
    }

    json::Array buses_list;
    const set<string>* buses = buses_ptr.value();

    for (const auto& bus : *buses) {
        buses_list.emplace_back(bus);
    }

    result = json::Builder{}
                    .StartDict()
                        .Key("request_id").Value(id)
                        .Key("buses").Value(move(buses_list))
                    .EndDict()
                .Build();

    return result;
}

const json::Node RequestHandler::HandleMapRequest(const json::Dict& request, MapRenderer& renderer) const {
    json::Node result;
    int id = request.at("id"s).AsInt();

    svg::Document svg_map = renderer.RenderMap();

    ostringstream strm;
    svg_map.Render(strm);

    result = json::Builder{}
                .StartDict()
                    .Key("request_id").Value(id)
                    .Key("map").Value(strm.str())
                .EndDict()
            .Build();

    return result;
}

const json::Node RequestHandler::HandleRoutingRequest(const json::Dict& request, const Router& router) const {
    json::Node result;
    int id = request.at("id"s).AsInt();
    const string_view from = request.at("from"s).AsString();
    const string_view to = request.at("to"s).AsString();
    const auto& route = router.FindRoute(from, to);

    if (!route.has_value()) {
        result = json::Builder{}
                    .StartDict()
                        .Key("request_id").Value(id)
                        .Key("error_message").Value("not found")
                    .EndDict()
                .Build();
    } else {
        json::Array items;
        double total_time = 0.0;
        graph::Router<double>::RouteInfo route_info = route.value();
        items.reserve(route_info.edges.size());
        
        for (auto& edge_id : route_info.edges) {
            const graph::Edge<double> edge = router.GetGraph().GetEdge(edge_id);
            if (edge.span_count == 0) {
                items.emplace_back(json::Node(json::Builder{}
                                    .StartDict()
                                        .Key("type").Value("Wait")
                                        .Key("stop_name").Value(edge.name)
                                        .Key("time").Value(edge.weight)
                                    .EndDict()
                                .Build()
                ));
            } else {
                items.emplace_back(json::Node(json::Builder{}
                                    .StartDict()
                                        .Key("type").Value("Bus")
                                        .Key("bus").Value(edge.name)
                                        .Key("span_count").Value(static_cast<int>(edge.span_count))
                                        .Key("time").Value(edge.weight)
                                    .EndDict()
                                .Build()
                ));
            }
            total_time += edge.weight;
        }

        result = json::Builder{}
                    .StartDict()
                        .Key("request_id").Value(id)
                        .Key("total_time").Value(total_time)
                        .Key("items").Value(move(items))
                    .EndDict()
                .Build();
    }

    return result;
}

} // namespace transport_catalogue::requesting