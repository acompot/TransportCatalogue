#include "serialization.h"

namespace serialize {

std::unique_ptr<transport::kernel::TransportCatalogue> Serializator::GetTransportCatalogue(){
  return std::move(rdb_);
}

std::unique_ptr<maprender::MapRender> Serializator::GetRenderer(){
  return std::move(renderer_);
}

std::unique_ptr<transport::Router> Serializator::GetRouter(){
  return std::move(router_);
}

serialize_info::Point Serializator::SerializePoint(const svg::Point& point) {
  serialize_info::Point result;
  result.set_x(point.x);
  result.set_y(point.y);
  return result;
}

serialize_info::Color Serializator::SerializeColor(const svg::Color& color){
  serialize_info::Color result;
  if (std::holds_alternative<std::string>(color)) {
    result.set_clr(std::get<std::string>(color));
  } else if (std::holds_alternative<svg::Rgb>(color)) {
    const auto &rgb = std::get<svg::Rgb>(color);
    serialize_info::RGB _rgb;
    _rgb.set_red(rgb.red);
    _rgb.set_green(rgb.green);
    _rgb.set_blue(rgb.blue);
    *result.mutable_rgb() = _rgb;
  } else if (std::holds_alternative<svg::Rgba>(color)) {
    auto &rgba = std::get<svg::Rgba>(color);
    serialize_info::RGBA _rgba;
    _rgba.set_red(rgba.red);
    _rgba.set_green(rgba.green);
    _rgba.set_blue(rgba.blue);
    _rgba.set_opacity(rgba.opacity);
    *result.mutable_rgba() = _rgba;
  }
  return result;
 }

bool Serializator::Serialize(const std::unique_ptr<jsonreader::JSONReader> &preader) {
  std::ofstream ofs(preader.get()->GetFilePath(), std::ios::binary);
  if (!ofs.is_open ()) {
    return false;
  }
  SaveStops(preader.get()->GetAllStops());
  SaveBuses(preader.get()->GetAllBuses());
  SaveRender(preader.get()->GetRenderInfo());
  SaveRouterSettings(preader);
  proto_catalogue_.SerializeToOstream(&ofs);
  return true;
}

bool Serializator::Deserialize(const std::string& path) {
  std::ifstream ifs(path, std::ios::binary);
  if (!ifs.is_open()  || !proto_catalogue_.ParseFromIstream(&ifs)) {
    return false;
  }
  rdb_ = std::make_unique<transport::kernel::TransportCatalogue>() ;
  LoadStop();
  LoadBus();
  std::vector<const transport::Stop*> tmp_stops(rdb_.get()->GetActualStops(sort_stops_.begin(),sort_stops_.end()));
  std::vector<const transport::Bus*> tmp_buses(rdb_.get()->GetActualBuses(sort_buses_.begin(),sort_buses_.end()));
  if (proto_catalogue_.has_render_settings()) {
    renderer_ = std::make_unique<maprender::MapRender>() ;
    auto render = LoadMapRender();
    renderer_.get()->CreateMap(rdb_,render,tmp_stops,tmp_buses);
  }
  if (proto_catalogue_.has_router()) {
    router_ = std::make_unique<transport::Router>();
    router_.get()->SetSettings(LoadRouterSettings());
    router_.get()->SetGraph(LoadGraph(),LoadStopIds());
  }
  return true;
}

void Serializator::SaveRender(const jsonreader::JSONRender& render){
  serialize_info::RenderSettings s_render;
  s_render.set_width(render.width);
  s_render.set_height(render.height);
  s_render.set_padding(render.padding);
  s_render.set_stop_radius(render.stop_radius);
  s_render.set_line_width(render.line_width);
  s_render.set_bus_label_font_size(render.bus_label_font_size);
  *s_render.mutable_bus_label_offset() = SerializePoint(render.bus_offset);
  s_render.set_stop_label_font_size(render.stop_label_font_size);
  *s_render.mutable_stop_label_offset() = SerializePoint(render.stop_offset);
  *s_render.mutable_underlayer_color() = SerializeColor(render.underlayer_color);
  s_render.set_underlayer_width(render.underlayer_width);
  for (const auto& c : render.palette) {
    *s_render.add_color_palette() = SerializeColor(c);
  }
  *proto_catalogue_.mutable_render_settings() = s_render;
}

void Serializator::SaveStops(const std::vector<jsonreader::JSONinfoStop>& stops) {
  for (const auto& ostop : stops) {
    serialize_info::Stop p_stop;
    p_stop.set_name(ostop.name_);
    p_stop.add_coordinate(ostop.lat);
    p_stop.add_coordinate(ostop.lng);
    for( const auto& [near,distans] :ostop.dist_){
      p_stop.add_near_stop(near);
      p_stop.add_distance(distans);
    }
    *proto_catalogue_.add_stop() = std::move(p_stop);
  }
}

void Serializator::SaveBuses(const std::vector<jsonreader::JSONinfoBus>& buses) {
  for (const auto& obus: buses){
    serialize_info::Bus p_bus;
    p_bus.set_name(obus.name_);
    for (const auto& s : obus.stops) {
      p_bus.add_stop(s);
    }
    p_bus.set_is_circle(obus.isCirle);
    *proto_catalogue_.add_bus() = std::move(p_bus);
  }
}

void Serializator::LoadStop() {
  for (int i = 0; i < proto_catalogue_.stop_size(); ++i) {
    const serialize_info::Stop& stop_i = proto_catalogue_.stop(i);
    rdb_.get()->AddStops(stop_i.name(),  stop_i.coordinate(0), stop_i.coordinate(1) );
    sort_stops_.insert(stop_i.name());
  }
  SetStopsDistances();
}

void Serializator::SetStopsDistances(){
  for (int i = 0; i < proto_catalogue_.stop_size(); ++i) {
    const serialize_info::Stop& stop_i = proto_catalogue_.stop(i);
    const transport::Stop* fr_ptr = rdb_.get()->GetStopStruct(stop_i.name());
    for (int j = 0; j < stop_i.near_stop_size(); ++j) {
      const transport::Stop* to_ptr = rdb_.get()->GetStopStruct(stop_i.near_stop(j));
      rdb_.get()->SetDistance(fr_ptr,to_ptr,stop_i.distance(j));
    }
  }
}

void Serializator::LoadBus(){
  for (int i = 0; i < proto_catalogue_.bus_size(); ++i) {
    const serialize_info::Bus& bus_i = proto_catalogue_.bus(i);
    std::vector<std::string> stops(bus_i.stop_size());
    for (size_t j = 0; j < stops.size(); ++j) {
      stops[j] = bus_i.stop(j);
    }
    rdb_.get()->AddBus(bus_i.name(), stops, bus_i.is_circle());
    sort_buses_.insert(bus_i.name());
  }
}

jsonreader::JSONRender Serializator::LoadMapRender(){
  const serialize_info::RenderSettings& render_from_db = proto_catalogue_.render_settings();
  jsonreader::JSONRender render;
  render.width = render_from_db.width();
  render.height = render_from_db.height();
  render.padding = render_from_db.padding();
  render.stop_radius = render_from_db.stop_radius();
  render.line_width = render_from_db.line_width();
  render.bus_label_font_size = render_from_db.bus_label_font_size();
  render.bus_offset = MakePoint(render_from_db.bus_label_offset());
  render.stop_label_font_size = render_from_db.stop_label_font_size();
  render.stop_offset =  MakePoint(render_from_db.stop_label_offset());
  render.underlayer_color = MakeColor(render_from_db.underlayer_color());
  render.underlayer_width  = render_from_db.underlayer_width();
  render.palette.reserve(render_from_db.color_palette().size());
  for (int i = 0; i < render_from_db.color_palette().size();++i){
     render.palette.push_back(MakeColor(render_from_db.color_palette(i)));
  }
  render.isRead = true;
  return render;

}

svg::Point Serializator::MakePoint(const serialize_info::Point &p_point) {
  svg::Point result;
  result.x = p_point.x();
  result.y = p_point.y();
  return result;
}

svg::Color Serializator::MakeColor(const serialize_info::Color &p_color) {
  svg::Color color;
  if (p_color.has_rgb()){
    color = svg::Rgb(p_color.rgb().red(), p_color.rgb().green(), p_color.rgb().blue());
  }else  if (p_color.has_rgba()){
    color = svg::Rgba(p_color.rgba().red(), p_color.rgba().green(), p_color.rgba().blue(),p_color.rgba().opacity());
  } else {
    color = p_color.clr();
  }
  return color;
}

void Serializator::SaveRouterSettings(const std::unique_ptr<jsonreader::JSONReader> &preader){
  serialize_info::Router p_router;
  serialize_info::RouterSettings r_settings;
  r_settings.set_bus_wait_time(preader.get()->GetRouteSettings().bus_wait_time_);
  r_settings.set_bus_velocity(preader.get()->GetRouteSettings().bus_velocity_);
  *p_router.mutable_router_settings() = r_settings;
  //Придется всё сохранять =(
  router_ = std::make_unique<transport::Router>();
  rdb_= std::make_unique<transport::kernel::TransportCatalogue>();
  LoadStop(preader);
  LoadBus(preader);
  std::vector<const transport::Stop*> tmp_stops(rdb_.get()->GetActualStops(sort_stops_.begin(),sort_stops_.end()));
  std::vector<const transport::Bus*> tmp_buses(rdb_.get()->GetActualBuses(sort_buses_.begin(),sort_buses_.end()));
  router_.get()->SetSettings(preader.get()->GetRouteSettings());
  router_.get()->BuildGraph(rdb_,tmp_stops,tmp_buses);
  *p_router.mutable_graph() = GetGraphSerialize(router_.get()->GetGraph());
  for (const auto& [n, id] : router_.get()->GetStopIds()) {
    serialize_info::StopId si;
    si.set_name(n);
    si.set_id(id);
    *p_router.add_stop_id() = si;
  }
  *proto_catalogue_.mutable_router() = p_router;
}

void Serializator::LoadStop(const std::unique_ptr<jsonreader::JSONReader> &preader) {
  for (const auto& stop : preader.get()->GetAllStops()){
    rdb_.get()->AddStops(stop.name_,  stop.lat, stop.lng );
    sort_stops_.insert(stop.name_);
  }
  SetStopsDistances(preader);
}

void Serializator::SetStopsDistances(const std::unique_ptr<jsonreader::JSONReader> &preader){
  for (const auto& stop: preader.get()->GetAllStops()) {
    const transport::Stop* fr_ptr = rdb_.get()->GetStopStruct(stop.name_);
    for (const auto& [name,dist] : stop.dist_) {
      const transport::Stop* to_ptr = rdb_.get()->GetStopStruct(name);
      rdb_.get()->SetDistance(fr_ptr,to_ptr,dist);
    }
  }
}

void Serializator::LoadBus(const std::unique_ptr<jsonreader::JSONReader> &preader){
  for (const auto& bus: preader.get()->GetAllBuses()){
    rdb_.get()->AddBus(bus.name_, bus.stops, bus.isCirle);
    sort_buses_.insert(bus.name_);
  }
}

serialize_info::Graph Serializator::GetGraphSerialize(const graph::DirectedWeightedGraph<double>& g) {
  serialize_info::Graph result;
  size_t vertex_count = g.GetVertexCount();
  size_t edge_count = g.GetEdgeCount();
  for (size_t i = 0; i < edge_count; ++i) {
    const graph::Edge<double>& edge = g.GetEdge(i);
    serialize_info::Edge s_edge;
    s_edge.set_name(edge.name);
    s_edge.set_quality(edge.quality);
    s_edge.set_from(edge.from);
    s_edge.set_to(edge.to);
    s_edge.set_weight(edge.weight);
    *result.add_edge() = s_edge;
  }
  for (size_t i = 0; i < vertex_count; ++i) {
    serialize_info::Vertex vertex;
    for (const auto& edge_id : g.GetIncidentEdges(i)) {
      vertex.add_edge_id(edge_id);
    }
    *result.add_vertex() = vertex;
  }
  return result;
}

graph::DirectedWeightedGraph<double> Serializator::LoadGraph() {
  const serialize_info::Graph& g = proto_catalogue_.router().graph();
  std::vector<graph::Edge<double>> edges(g.edge_size());
  std::vector<std::vector<graph::EdgeId>> incidence_lists(g.vertex_size());
  for (size_t i = 0; i < edges.size(); ++i) {
    const serialize_info::Edge& e = g.edge(i);
    edges[i] = { e.name(), static_cast<size_t>(e.quality()),
    static_cast<size_t>(e.from()), static_cast<size_t>(e.to()), e.weight() };
  }
  for (size_t i = 0; i < incidence_lists.size(); ++i) {
    const serialize_info::Vertex& v = g.vertex(i);
    incidence_lists[i].reserve(v.edge_id_size());
    for (const auto& id : v.edge_id()) {
      incidence_lists[i].push_back(id);
    }
  }
  return graph::DirectedWeightedGraph<double>(edges, incidence_lists);
}

transport::RouteSettings Serializator::LoadRouterSettings(){
  const serialize_info::RouterSettings& rs = proto_catalogue_.router().router_settings();
  return transport::RouteSettings{rs.bus_wait_time(),rs.bus_velocity()};
}

std::map<std::string, graph::VertexId> Serializator::LoadStopIds() {
    std::map<std::string, graph::VertexId> result;
    for (const auto& s :  proto_catalogue_.router().stop_id()) {
        result[s.name()] = s.id();
    }
    return result;
}

} // namespace serialize
