#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <fstream>
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"
#include "svg.h"
#include "transport_catalogue.pb.h"

namespace serialize {

class Serializator {
public:
  using ProtoTransportCatalogue = serialize_info::TransportCatalogue;
  Serializator()  =default;
  std::unique_ptr<transport::kernel::TransportCatalogue> GetTransportCatalogue();
  std::unique_ptr<maprender::MapRender> GetRenderer();
  std::unique_ptr<transport::Router> GetRouter();
  // сохраняет данные транспортного каталога в бинарном виде в соответсвии с настройками
  bool Serialize(const std::unique_ptr<jsonreader::JSONReader>& preader);
  // загружает данные в транспортный каталог из файла в соответствии с настройками
  bool Deserialize(const std::string& path);
private:
  void SaveRenderSettings(const jsonreader::JSONRender &settings);
  void SaveStops(const std::vector<jsonreader::JSONinfoStop>& stops);
  void SaveBuses(const std::vector<jsonreader::JSONinfoBus>& buses);
  void SaveRender(const jsonreader::JSONRender& render);
  void SaveRouterSettings(const std::unique_ptr<jsonreader::JSONReader>& preader);
  serialize_info::Graph GetGraphSerialize(const graph::DirectedWeightedGraph<double>& g);
  serialize_info::Point SerializePoint(const svg::Point &point);
  serialize_info::Color SerializeColor(const svg::Color &color);
  std::filesystem::path path_settings_;
  ProtoTransportCatalogue proto_catalogue_;
  void LoadStop();
  void LoadBus();
  void SetStopsDistances();
  void LoadStop(const std::unique_ptr<jsonreader::JSONReader> &preader);
  void LoadBus(const std::unique_ptr<jsonreader::JSONReader> &preader);
  void SetStopsDistances(const std::unique_ptr<jsonreader::JSONReader> &preader);
  graph::DirectedWeightedGraph<double> LoadGraph();
  transport::RouteSettings LoadRouterSettings();
  jsonreader::JSONRender LoadMapRender();
  std::map<std::string, graph::VertexId> LoadStopIds();
  svg::Point MakePoint(const serialize_info::Point& p_point);
  svg::Color MakeColor(const serialize_info::Color& p_color);
  std::unique_ptr<transport::kernel::TransportCatalogue> rdb_=nullptr;
  std::unique_ptr<maprender::MapRender> renderer_=nullptr;
  std::unique_ptr<transport::Router> router_=nullptr;
  std::set<std::string> sort_buses_;
  std::set<std::string> sort_stops_;
};

} // namespace serialize
