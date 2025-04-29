#include "map_renderer.h"

#include <string>

using namespace std;

namespace transport_catalogue::map_renderer {

bool IsZero(double value) {
    return std::abs(value) < EPSILON;
}

vector<Coordinates> MapRenderer::CollectStopCoords(const set<const Bus*, BusNameComparator>& buses) const {
    vector<Coordinates> stops_coords;
    for (const auto* bus : buses) {
        for (const auto* stop : bus->stops) {
            stops_coords.emplace_back(stop->coords);
        }
    }
    return stops_coords;
}

unique_ptr<SphereProjector> MapRenderer::CreateProjector(const vector<Coordinates>& stops_coords) const {
    return std::make_unique<SphereProjector>(
        stops_coords.begin(), stops_coords.end(),
        settings_.width, settings_.height, settings_.padding);
}

void MapRenderer::RenderRoutePolylines(const set<const Bus*, BusNameComparator>& buses, Document& doc) const {
    size_t color_num = 0;

    for (const auto* bus : buses) {
        if (bus->stops.empty()) {
            continue;
        }

        vector<const Stop*> bus_stops {bus->stops.begin(), bus->stops.end()};
        Polyline polyline;

        for (const auto* stop : bus_stops) {
            polyline.AddPoint((*projector_)(stop->coords));
        }

        polyline.SetFillColor(NoneColor)
                .SetStrokeColor(settings_.color_palette[color_num])
                .SetStrokeWidth(settings_.line_width)
                .SetStrokeLineCap(StrokeLineCap::ROUND)
                .SetStrokeLineJoin(StrokeLineJoin::ROUND);
        
        if (color_num < settings_.color_palette.size() - 1) {
            ++color_num;
        } else {
            color_num = 0;
        }
        
        doc.Add(polyline);
    }
}

void MapRenderer::RenderRouteLabels(const set<const Bus*, BusNameComparator>& buses, Document& doc) const {
    size_t color_num = 0;

    for (const auto* bus : buses) {
        if (bus->stops.empty()) {
            continue;
        }

        vector<const Stop*> route_end_stops;
        const Stop* first_stop = bus->stops.front();
        route_end_stops.push_back(first_stop);
        if (!bus->is_circular) {
            const Stop* last_stop = bus->stops[bus->stops.size() / 2];
            route_end_stops.push_back(last_stop);
            if (first_stop == last_stop) {
                route_end_stops.pop_back();
            }
        }

        for (const auto* stop : route_end_stops) {
            Point stop_point = (*projector_)(stop->coords);
            
            Text background_text;
            background_text.SetPosition(stop_point)
                        .SetOffset(settings_.bus_label_offset)
                        .SetFontSize(settings_.bus_label_font_size)
                        .SetFontFamily("Verdana"s)
                        .SetFontWeight("bold"s)
                        .SetFillColor(settings_.underlayer_color)
                        .SetStrokeColor(settings_.underlayer_color)
                        .SetStrokeWidth(settings_.underlayer_width)
                        .SetStrokeLineCap(StrokeLineCap::ROUND)
                        .SetStrokeLineJoin(StrokeLineJoin::ROUND)
                        .SetData(bus->name);
            doc.Add(background_text);

            Text label_text;
            label_text.SetPosition(stop_point)
                    .SetOffset(settings_.bus_label_offset)
                    .SetFontSize(settings_.bus_label_font_size)
                    .SetFontFamily("Verdana"s)
                    .SetFontWeight("bold"s)
                    .SetFillColor(settings_.color_palette[color_num])
                    .SetData(bus->name);
            doc.Add(label_text);
        }

        if (color_num < settings_.color_palette.size() - 1) {
            ++color_num;
        } else {
            color_num = 0;
        }
    }
}

void MapRenderer::RenderStopSymbols(const set<const Stop*, StopNameComparator>& stops, Document& doc) const {
    for (const auto* stop : stops) {
        Circle circle;
        Point stop_point = (*projector_)(stop->coords);
        circle.SetCenter(stop_point)
            .SetRadius(settings_.stop_radius)
            .SetFillColor("white"s);
        doc.Add(circle);
    }
}

void MapRenderer::RenderStopsLabels(const set<const Stop*, StopNameComparator>& stops, Document& doc) const {
    for (const auto* stop : stops) {
        Point stop_point = (*projector_)(stop->coords);
        
        Text background_text;
        background_text.SetPosition(stop_point)
                    .SetOffset(settings_.stop_label_offset)
                    .SetFontSize(settings_.stop_label_font_size)
                    .SetFontFamily("Verdana"s)
                    .SetFillColor(settings_.underlayer_color)
                    .SetStrokeColor(settings_.underlayer_color)
                    .SetStrokeWidth(settings_.underlayer_width)
                    .SetStrokeLineCap(StrokeLineCap::ROUND)
                    .SetStrokeLineJoin(StrokeLineJoin::ROUND)
                    .SetData(stop->name);
        doc.Add(background_text);

        Text label_text;
        label_text.SetPosition(stop_point)
                .SetOffset(settings_.stop_label_offset)
                .SetFontSize(settings_.stop_label_font_size)
                .SetFontFamily("Verdana"s)
                .SetFillColor("black"s)
                .SetData(stop->name);
        doc.Add(label_text);
    }
}

Document MapRenderer::RenderMap() {
    Document doc;
    
    auto all_buses = catalogue_.GetAllBuses();
    if (!all_buses.has_value()) {
        return doc;
    }

    auto all_stops = catalogue_.GetAllStops();
    if (!all_stops.has_value()) {
        return doc;
    }

    const set<const Bus*, BusNameComparator> buses = all_buses.value();
    const set<const Stop*, StopNameComparator> stops = all_stops.value();
    const auto stops_coords = CollectStopCoords(buses);
    projector_ = CreateProjector(stops_coords);
    RenderRoutePolylines(buses, doc);
    RenderRouteLabels(buses, doc);
    RenderStopSymbols(stops, doc);
    RenderStopsLabels(stops, doc);
    
    return doc;
}

} // namespace transport_catalogue::map_renderer