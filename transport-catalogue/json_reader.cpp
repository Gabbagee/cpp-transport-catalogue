#include "json_reader.h"
#include "svg.h"

#include <string>
#include <string_view>
#include <vector>

using namespace std;

namespace transport_catalogue::processing {

JsonReader::JsonReader(TransportCatalogue& catalogue)
    : catalogue_(catalogue)
    , handler_(catalogue) {}

void JsonReader::ProcessBaseRequests(const json::Document& doc) {
    const json::Node& root = doc.GetRoot().AsMap().at("base_requests"s);
    const auto& base_requests = root.AsArray();

    for (const auto& request : base_requests) {
        const auto& stop_map = request.AsMap();
        const auto& type = stop_map.at("type"s).AsString();

        if (type == "Stop"s) {
            const auto& name = stop_map.at("name"s).AsString();
            double latitude = stop_map.at("latitude"s).AsDouble();
            double longitude = stop_map.at("longitude"s).AsDouble();
            catalogue_.AddStop(name, {latitude, longitude});
        }
    }
    
    for (const auto& request : base_requests) {
        const auto& stop_map = request.AsMap();
        const auto& type = stop_map.at("type"s).AsString();

        if (type == "Stop"s) {
            const auto& name = stop_map.at("name"s).AsString();
            const Stop* stop_info = catalogue_.GetStopInfo(name).value();
            const auto& dists = stop_map.at("road_distances"s).AsMap();

            for (const auto& [dest_name, dist_node] : dists) {
                const int dist = dist_node.AsInt();
                const auto* dest_info = catalogue_.GetStopInfo(dest_name).value();
                catalogue_.SetDistance(stop_info, dest_info, dist);
                if (catalogue_.GetDistance(dest_info, stop_info) == 0) {
                    catalogue_.SetDistance(dest_info, stop_info, dist);
                }
            }
        }
    }

    for (const auto& request : base_requests) {
        const auto& bus_map = request.AsMap();
        const auto& type = bus_map.at("type"s).AsString();

        if (type == "Bus"s) {
            const auto& name = bus_map.at("name"s).AsString();
            const auto& stops = bus_map.at("stops"s).AsArray();
            vector<string_view> stops_names;

            for (const auto& stop_node : stops) {
                stops_names.push_back(stop_node.AsString());
            }
            
            bool is_circular = bus_map.at("is_roundtrip"s).AsBool();
            catalogue_.AddRoute(name, stops_names, is_circular);
        }    
    }
}

json::Array JsonReader::ProcessStatRequests(const json::Document& doc, MapRenderer& renderer, const Router& router) const {
    const json::Node& root = doc.GetRoot().AsMap().at("stat_requests"s);
    const auto& stat_requests = root.AsArray();
    json::Array result;

    for (const auto& request : stat_requests) {
        const auto& request_map = request.AsMap();
        const string& type = request_map.at("type"s).AsString();
        
        if (type == "Stop"s) {
            result.push_back(handler_.HandleStopRequest(request_map));
        } else if (type == "Bus"s) {
            result.push_back(handler_.HandleRouteRequest(request_map));
        } else if (type == "Map"s) {
            result.push_back(handler_.HandleMapRequest(request_map, renderer));
        } else if (type == "Route"s) {
            result.push_back(handler_.HandleRoutingRequest(request_map, router));
        }
    }
    return result;
}

RenderSettings JsonReader::ProcessRenderSettings(const json::Document& doc) const {
    const json::Node& root = doc.GetRoot().AsMap().at("render_settings"s);
    const auto& render_settings = root.AsMap();
    RenderSettings result;

    result.width = render_settings.at("width"s).AsDouble();
    result.height = render_settings.at("height"s).AsDouble();
    result.padding = render_settings.at("padding"s).AsDouble();
    result.line_width = render_settings.at("line_width"s).AsDouble();
    result.stop_radius = render_settings.at("stop_radius"s).AsDouble();
    result.bus_label_font_size = static_cast<uint32_t>(render_settings.at("bus_label_font_size"s).AsInt());
    result.stop_label_font_size = static_cast<uint32_t>(render_settings.at("stop_label_font_size"s).AsInt());
    const auto& bus_label_offset = render_settings.at("bus_label_offset"s).AsArray();
    result.bus_label_offset = {bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble()};
    const auto& stop_label_offset = render_settings.at("stop_label_offset"s).AsArray();
    result.stop_label_offset = {stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble()};

    const auto& underlayer_color = render_settings.at("underlayer_color"s);
    if (underlayer_color.IsString()) {
        result.underlayer_color = underlayer_color.AsString();
    } else if (underlayer_color.IsArray()) {
        const auto& color_array = underlayer_color.AsArray();
        if (color_array.size() == 3) {
            result.underlayer_color = svg::Rgb{
                static_cast<uint8_t>(color_array[0].AsInt()),
                static_cast<uint8_t>(color_array[1].AsInt()),
                static_cast<uint8_t>(color_array[2].AsInt())
            };
        } else if (color_array.size() == 4) {
            result.underlayer_color = svg::Rgba{
                static_cast<uint8_t>(color_array[0].AsInt()),
                static_cast<uint8_t>(color_array[1].AsInt()),
                static_cast<uint8_t>(color_array[2].AsInt()),
                color_array[3].AsDouble()
            };
        }
    } else throw logic_error("Invalid underlayer color format");

    result.underlayer_width = render_settings.at("underlayer_width"s).AsDouble();
    
    const auto& color_palette = render_settings.at("color_palette"s).AsArray();
    for (const auto& color : color_palette) {
        if (color.IsString()) {
            result.color_palette.emplace_back(color.AsString());
        } else if (color.IsArray()) {
            const auto& color_array = color.AsArray();
            if (color_array.size() == 3) {
                result.color_palette.emplace_back(svg::Rgb{
                    static_cast<uint8_t>(color_array[0].AsInt()),
                    static_cast<uint8_t>(color_array[1].AsInt()),
                    static_cast<uint8_t>(color_array[2].AsInt())
                });
            } else if (color_array.size() == 4) {
                result.color_palette.emplace_back(svg::Rgba{
                    static_cast<uint8_t>(color_array[0].AsInt()),
                    static_cast<uint8_t>(color_array[1].AsInt()),
                    static_cast<uint8_t>(color_array[2].AsInt()),
                    color_array[3].AsDouble()
                });
            }
        } else throw logic_error("Invalid color palette format");
    }
    return result;
}

RoutingSettings JsonReader::ProcessRouterSettings(const json::Document& doc) const {
    const json::Node& root = doc.GetRoot().AsMap().at("routing_settings"s);
    const auto& routing_settings = root.AsMap();
    RoutingSettings result;

    result.bus_velocity = routing_settings.at("bus_velocity"s).AsDouble();
    result.bus_wait_time = routing_settings.at("bus_wait_time"s).AsInt();

    return result;
}

void JsonReader::ProcessDocument(const json::Document& doc, ostream& output) {
    ProcessBaseRequests(doc);
    RenderSettings render_settings = ProcessRenderSettings(doc);
    RoutingSettings routing_settings = ProcessRouterSettings(doc);

    MapRenderer renderer(render_settings, catalogue_);
    Router router(routing_settings, catalogue_);

    json::Array stat_responses = ProcessStatRequests(doc, renderer, router);

    json::Print(json::Document(stat_responses), output);
}

} // namespace transport_catalogue::processing