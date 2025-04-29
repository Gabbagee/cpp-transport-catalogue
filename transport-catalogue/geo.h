#pragma once

namespace transport_catalogue::geo {

struct Coordinates {
    double lat;
    double lng;

    bool operator==(const Coordinates& other) const;

    bool operator!=(const Coordinates& other) const;
};

inline constexpr double EARTH_RADIUS = 6371000.;

double ComputeDistance(Coordinates from, Coordinates to);

}  // namespace transport_catalogue::geo