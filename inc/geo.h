#pragma once

#include <cmath>
#include <iomanip>
#include <iostream>
namespace geo {

  const static uint32_t EarthRadius =6371000;

  struct Coordinates {
    double lat = .0;
    double lng = .0;

     bool operator==(const Coordinates& rhs) const;
  };

  double ComputeDistance(Coordinates from, Coordinates to);
}
