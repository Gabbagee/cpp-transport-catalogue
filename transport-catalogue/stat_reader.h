#pragma once

#include "transport_catalogue.h"

#include <iosfwd>
#include <string_view>

namespace transport_catalogue::retrieving {

using namespace transport_catalogue::database;

void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
                       std::ostream& output);

}  // namespace transport_catalogue::retrieving