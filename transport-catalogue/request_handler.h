#pragma once

#include "json.h"
#include "map_renderer.h"
#include "svg.h"
#include "transport_catalogue.h"
#include "transport_router.h"

#include <optional>

namespace transport_catalogue::requesting {

using namespace transport_catalogue::database;
using namespace transport_catalogue::map_renderer;
using namespace transport_catalogue::routing;

class RequestHandler {
public:
    explicit RequestHandler(const TransportCatalogue& catalogue);

    const json::Node HandleRouteRequest(const json::Dict& request) const;

    const json::Node HandleStopRequest(const json::Dict& request) const;

    const json::Node HandleMapRequest(const json::Dict& request, MapRenderer& renderer) const;

    const json::Node HandleRoutingRequest(const json::Dict& request, const Router& router) const;

private:
    const TransportCatalogue& catalogue_;
};

} // namespace transport_catalogue::requesting