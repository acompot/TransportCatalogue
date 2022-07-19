#include "json_reader.h"

namespace jsonreader {

using namespace std::literals;

void JSONReader::Read(std::unique_ptr<json::Document> &ptr){
  //Корневой раздел
  auto doc = ptr.get();
  auto node = doc->GetRoot();
   
   JSONinfoBus bus;
   bool f = false;
   std::vector<JSONinfoBus>tmp_buses;
  //Ищу base_request или stat_request;
  const json::Dict& dict = node.AsDict();
  //парсю bas_request и stat_request
  if (dict.count("base_requests")) {
      for (const auto& base  : dict.at("base_requests").AsArray()){
      const json::Dict& about_bus_stop  = base.AsDict();
      //Разово резервирую место
      if(f == false){
        json_buses_.reserve(about_bus_stop.size());
        json_stops_.reserve(about_bus_stop.size());
        f = true;
      }
      if(about_bus_stop.at("type").AsString() == "Bus") {
        bus.name_ =about_bus_stop.at("name").AsString();
//        sort_buses_.insert(bus.name_);
        bus.isCirle = about_bus_stop.at("is_roundtrip").AsBool();
        const json::Array& bus_stops = about_bus_stop.at("stops").AsArray();
        bus.stops.reserve(bus_stops.size());
        for (const auto& stop : bus_stops){
          bus.stops.push_back(stop.AsString());
        }
        json_buses_.push_back(bus);
        bus={};
      }
      else if (about_bus_stop.at("type").AsString() == "Stop") ParseStopsRequest(about_bus_stop);
    }
  }
  if (dict.count("render_settings")) ParseRenderRequest(dict.at("render_settings"));
  if (dict.count("stat_requests"))   ParseStateRequest(dict.at("stat_requests"));
  if (dict.count("routing_settings")) ParseRouteSettings(dict.at("routing_settings"));
  if (dict.count("serialization_settings")){
      save_path_db_ =  dict.at("serialization_settings").AsDict().at("file"s).AsString();
  }
//  if (!json_stops_.empty()){
//    std::sort(json_stops_.begin(),json_stops_.end(),[](const auto lsh,const auto rhs){
//       return lsh.name_ < rhs.name_;
//    });
// }
//  if (!json_buses_.empty()){
//    std::sort(json_buses_.begin(),json_buses_.end(),[](const auto lsh,const auto rhs){
//       return lsh.name_ < rhs.name_;
//    });
// }
//  //Когда все остановки добавлены, добавлю и автобусы
//  //Считаю расстояния
////  CalculateDistTC(ptc);
////  ParseBusRequest(tmp_buses,ptc);



}

void JSONReader::ParseRenderRequest(const json::Node& node){
  render_info_.isRead = true;
  for (const auto& [key,value] : node.AsDict()){
    if (key == "width") render_info_.width = value.AsDouble();
    if (key == "height") render_info_.height = value.AsDouble();
    if (key == "padding")render_info_.padding = value.AsDouble();
    if (key == "line_width") render_info_.line_width = value.AsDouble();
    if (key == "stop_radius") { render_info_.stop_radius = value.AsDouble();}
    if (key == "bus_label_font_size") render_info_.bus_label_font_size = value.AsInt();
    if (key == "bus_label_offset") SetBusOffset(value);
    if (key == "stop_label_font_size") render_info_.stop_label_font_size = value.AsInt();
    if (key == "stop_label_offset") SetLabelOffset(value);
    if (key == "underlayer_color")GetUnderlayerColor(value);
    if (key ==  "underlayer_width") render_info_.underlayer_width = value.AsDouble();
    if (key == "color_palette") CreatePalette(value);

  }
}

void JSONReader::SetBusOffset(const json::Node& node){
  //Согласно протоколу [0]-dx,[1]-dy
  auto t = node.AsArray();
  render_info_.bus_offset.x = t[0].AsDouble();
  render_info_.bus_offset.y = t[1].AsDouble();
}

void JSONReader::SetLabelOffset(const json::Node& node){
  //Согласно протоколу [0]-dx,[1]-dy
  auto t = node.AsArray();
  render_info_.stop_offset.x = t[0].AsDouble();
  render_info_.stop_offset.y = t[1].AsDouble();
}

void JSONReader::GetUnderlayerColor(const json::Node& node){
    //цвет
    if (node.IsString()) render_info_.underlayer_color = node.AsString();
    else if (node.AsArray().size() == 3){
      auto t = node.AsArray();
      uint8_t r = static_cast<uint8_t>(t[0].AsInt());
      uint8_t g = static_cast<uint8_t>(t[1].AsInt());
      uint8_t b = static_cast<uint8_t>(t[2].AsInt());
      render_info_.underlayer_color = svg::Rgb{r,g,b};
    } else {
      auto t = node.AsArray();
      uint8_t r = static_cast<uint8_t>(t[0].AsInt());
      uint8_t g = static_cast<uint8_t>(t[1].AsInt());
      uint8_t b =static_cast<uint8_t>(t[2].AsInt());
      double o =  t[3].AsDouble();
      render_info_.underlayer_color = svg::Rgba{r,g,b,o};
    }
}

void JSONReader::CreatePalette(const json::Node& node){
  for (const auto& pal : node.AsArray()){
    render_info_.palette.reserve(node.AsArray().size());
    if (pal.IsString()) render_info_.palette.push_back(pal.AsString());
    else if (pal.AsArray().size() == 3) {
      auto t = pal.AsArray();
      uint8_t r = static_cast<uint8_t>(t[0].AsInt());
      uint8_t g = static_cast<uint8_t>(t[1].AsInt());
      uint8_t b = static_cast<uint8_t>(t[2].AsInt());
      render_info_.palette.push_back(svg::Rgb{r,g,b});
    } else {
      auto t = pal.AsArray();
      uint8_t r = static_cast<uint8_t>(t[0].AsInt());
      uint8_t g = static_cast<uint8_t>(t[1].AsInt());
      uint8_t b =static_cast<uint8_t>(t[2].AsInt());
      double o = pal.AsArray()[3].AsDouble();
      render_info_.palette.push_back(svg::Rgba{r,g,b,o});
    }
  }
}

void JSONReader::ParseBusRequest(std::vector<JSONinfoBus> buses, const std::unique_ptr<transport::kernel::TransportCatalogue>& ptc){
  auto tc = ptc.get();
  for (JSONinfoBus& bus :buses){
    tc->AddBus(bus.name_,bus.stops,bus.isCirle);

  }
}

void JSONReader::ParseStopsRequest(const json::Dict& stops){
//  auto tc = ptc.get();
  JSONinfoStop stops_;
  std::map<std::string,int>names_and_dist;
  stops_.name_= stops.at("name").AsString();
//  sort_stops_.insert(stops_.name_);
  stops_.lat = stops.at("latitude").AsDouble();
  stops_.lng = stops.at("longitude").AsDouble();
  if ( stops.count("road_distances")){
    for ( const auto& [key,value] : stops.at("road_distances").AsDict()){
     names_and_dist[key] = value.AsInt();
    }
  }

  stops_.dist_ = std::move(names_and_dist);
  names_and_dist.clear();
  json_stops_.emplace_back(stops_);
}

void JSONReader::ParseStateRequest(const json::Node& stat)  {
  StateRequest st;
  state_requests_.reserve(stat.AsArray().size());
  auto vec_req = stat.AsArray();
  int stat_size = static_cast<int>(vec_req.size());
  for (int i = 0; i < stat_size;++i){
    if (vec_req[i].AsDict().count("name")) st.name_ = std::move(vec_req[i].AsDict().at("name").AsString());
    if (vec_req[i].AsDict().count("type")) st.type = std::move(vec_req[i].AsDict().at("type").AsString());
    st.id = vec_req[i].AsDict().at("id").AsInt();
    st.position = i;
    if (st.type == "Route"){
         if (vec_req[i].AsDict().count("from")) st.from_ = std::move(vec_req[i].AsDict().at("from").AsString());
         if (vec_req[i].AsDict().count("to")) st.to_ = std::move(vec_req[i].AsDict().at("to").AsString());
    }
    state_requests_.emplace_back(st);
  }
}

void  JSONReader::ParseRouteSettings(const json::Node& stat)  {

  for (const auto& [key,value] : stat.AsDict()){
    if (key == "bus_wait_time") RouteSettings.bus_wait_time_ = value.AsInt();
    if (key == "bus_velocity")  RouteSettings.bus_velocity_  = value.AsDouble();
  }


}

std::vector<JSONinfoBus> JSONReader::GetAllBuses() const{
    return json_buses_;
}
std::vector<JSONinfoStop> JSONReader::GetAllStops() const{
    return json_stops_;
}

std::vector<StateRequest> JSONReader::GetStateRequest() const{return state_requests_;}

JSONRender JSONReader::GetRenderInfo() const {
    return render_info_;
}

void JSONReader::PrintJSON(const json::Node& node,std::ostream& out) const{
  auto outj = std::make_unique<json::Document>(node);
  auto doc = *outj.get();
  json::Print(doc,out);
  outj.release();
}

void JSONReader::PrintJSON(const json::Document& doc,std::ostream& out) const{
  json::Print(doc,out);
}

std::set<std::string> JSONReader::GetSortNames() const{
  return sort_buses_;
}

std::set<std::string> JSONReader::GetStopSortNames() const{
  return sort_stops_;
}

void  JSONReader::ClearNames(){
  sort_buses_.clear();
}

void  JSONReader::ClearStops(){
  sort_stops_.clear();
}

transport::RouteSettings JSONReader::GetRouteSettings() const{
    transport::RouteSettings tmp;
    tmp.bus_wait_time_ = RouteSettings.bus_wait_time_;
    tmp.bus_velocity_  = RouteSettings.bus_velocity_;
    return tmp;
}

std::string JSONReader::GetFilePath() const{
    return save_path_db_;
}
}//namespace jsonreader
