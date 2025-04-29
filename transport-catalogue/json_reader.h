#pragma once

#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

#include <iostream>

namespace transport_catalogue::processing {

using namespace transport_catalogue::database;
using namespace transport_catalogue::requesting;
using namespace transport_catalogue::map_renderer;

class JsonReader {
public:
    JsonReader(TransportCatalogue& catalogue);

    void ProcessBaseRequests(const json::Document& doc);

    json::Array ProcessStatRequests(const json::Document& doc, MapRenderer& renderer) const;

    RenderSettings ProcessRenderSettings(const json::Document& doc) const;

    void ProcessDocument(const json::Document& doc, std::ostream& output);

private:
    TransportCatalogue& catalogue_;
    RequestHandler handler_;
};

} // namespace transport_catalogue::processing