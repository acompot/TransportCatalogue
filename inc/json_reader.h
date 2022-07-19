#pragma once
#include "json.h"
#include "svg.h"
#include  "transport_catalogue.h"
#include <filesystem>


namespace jsonreader {

struct JSONinfoBus{
  std::string name_;
  std::vector<std::string> stops;
  bool isCirle;
};

struct JSONinfoStop{
  using StopsWithDist =std::map<std::string,int> ;
  std::string name_;
  double lat;
  double lng;
  StopsWithDist dist_;

};

struct StateRequest{
  std::string type;
  std::string name_;
  std::string from_;
  std::string to_;
  int id;
  int position;//Для кэширования
};

struct JSONRender{
  bool isRead = false;
  double width;
  double height;
  double padding;
  double line_width;
  double stop_radius;
  int bus_label_font_size;
  svg::Point bus_offset;
  int stop_label_font_size;
  svg::Point stop_offset;
  svg::Color underlayer_color;
  double underlayer_width;//stroke_witdh
  std::vector<svg::Color> palette;
};

class JSONReader{
public:
  JSONReader()=default;

  void Read(std::unique_ptr<json::Document> &ptr);
  std::set<std::string> GetSortNames() const;
  std::set<std::string> GetStopSortNames() const;
  void  ClearNames();
  void  ClearStops();
  std::vector<StateRequest> GetStateRequest() const;
  std::vector<JSONinfoBus>  GetAllBuses() const;
  std::vector<JSONinfoStop> GetAllStops() const;
  JSONRender GetRenderInfo() const;
  void PrintJSON(const json::Node& node, std::ostream &out) const;
  void PrintJSON(const json::Document& doc,std::ostream &out) const;
  transport::RouteSettings GetRouteSettings() const;
  std::string GetFilePath() const;



private:

  struct JSONRouteSettings {
      JSONRouteSettings() = default;
      JSONRouteSettings(int time,double speed);
      int bus_wait_time_{};
      double bus_velocity_{};
  }RouteSettings;

  void ParseStateRequest(const json::Node& stat);
  void ParseBusRequest(std::vector<JSONinfoBus> bus,  const std::unique_ptr<transport::kernel::TransportCatalogue> &ptc);
  void ParseRouteSettings(const json::Node& stat);
  void ParseStopsRequest(const json::Dict &stops);
  void ParseRenderRequest(const json::Node& node);
  void CalculateDistTC(const std::unique_ptr<transport::kernel::TransportCatalogue> &ptc);
  void CreatePalette(const json::Node& node);
  void GetUnderlayerColor(const json::Node& node);
  void SetBusOffset(const json::Node& node);
  void SetLabelOffset(const json::Node& node);
  std::set<std::string> sort_buses_;
  std::set<std::string> sort_stops_;
  std::vector<StateRequest> state_requests_;
  JSONRender render_info_{};
  std::vector<JSONinfoBus>json_buses_;
  std::vector<JSONinfoStop> json_stops_;
  std::string save_path_db_;

  std::unordered_map<std::string,std::map<std::string,double>> weight_;



};

}//namespace jsonreader
