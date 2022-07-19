#pragma once
#include "svg.h"
#include "transport_catalogue.h"
#include "json_reader.h"

namespace maprender {

inline const double EPSILON = 1e-6;
bool IsZero(double value);
class SphereProjector {
public:
  template <typename PointInputIt>
  SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width,
                    double max_height, double padding)
        : padding_(padding) {
    if (points_begin == points_end) {
      return;
    }

        const auto [left_it, right_it]
            = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
                  return lhs.lng < rhs.lng;
              });
        min_lon_ = left_it->lng;
        const double max_lon = right_it->lng;

        const auto [bottom_it, top_it]
            = std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
                  return lhs.lat < rhs.lat;
              });
        const double min_lat = bottom_it->lat;
        max_lat_ = top_it->lat;

        std::optional<double> width_zoom;
        if (!IsZero(max_lon - min_lon_)) {
            width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
        }

        std::optional<double> height_zoom;
        if (!IsZero(max_lat_ - min_lat)) {
            height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
        }

        if (width_zoom && height_zoom) {
            zoom_coeff_ = std::min(*width_zoom, *height_zoom);
        } else if (width_zoom) {
            zoom_coeff_ = *width_zoom;
        } else if (height_zoom) {
            zoom_coeff_ = *height_zoom;
        }
    }

    svg::Point operator()(geo::Coordinates coords) const {
        return {(coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
    }

private:
    double padding_;
    double min_lon_ = 0;
    double max_lat_ = 0;
    double zoom_coeff_ = 0;
};

class MapRender{
public:
  MapRender() = default;
  void CreateMap(const std::unique_ptr<transport::kernel::TransportCatalogue> &ptc
                 , const jsonreader::JSONRender& render
                 , std::vector<const transport::Stop*>& actual_stops
                 , std::vector<const transport::Bus*>& get_all_buses);
  void Print(std::ostream& out) const;
  void Print() const;
private:
  std::vector<svg::Document> picture;
  svg::Document doc{};
  std::vector<svg::Polyline> polylines_;
  std::vector<svg::Text> texts_;
  std::vector<svg::Circle> cirles_;
  std::vector<svg::Text> name_stops_;
  std::vector<geo::Coordinates> CreateCoordCont(const std::vector<const transport::Stop *>& stops) const;
  svg::Polyline CreatePolyline(const std::vector<svg::Point>& points, double line_width, svg::Color color) const;
  svg::Text     CreateText(const svg::Point point, const jsonreader::JSONRender &rendr, const std::string& name, svg::Color color, bool isStop =false);
  svg::Text     CreateText(const svg::Point point, const jsonreader::JSONRender &rendr, const std::string& name, bool isStop =false);
  svg::Text     CreateText(const svg::Point point, const jsonreader::JSONRender &rendr, const std::string& name) const;
  svg::Circle CreateCircle(const svg::Point point, double radius) const;
  void BusPart(const std::vector<const transport::Bus *> &all_buses, const maprender::SphereProjector& sp, const jsonreader::JSONRender &rendr);
  void StopPart(const std::unique_ptr<transport::kernel::TransportCatalogue>& ptc,const std::vector<const transport::Stop*>& stops, const maprender::SphereProjector& sp, const jsonreader::JSONRender &rendr);
  void CreateTextCircle(const transport::Bus *bus, svg::Point& first, svg::Point& last, int position, const jsonreader::JSONRender& render);

};

}//namespace maprender


