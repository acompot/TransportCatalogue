#include "request_handler.h"

namespace requesthandler {

RequestHandler& RequestHandler::Read(std::unique_ptr<json::Document> &ptr){
  if (!reader_) reader_ = std::make_unique<jsonreader::JSONReader>() ;
  reader_.get()->Read(ptr);
  return *this;
}

void RequestHandler::SendJson (std::ostream &out){
  using namespace std::literals;
  std::vector<json::Node> outvec;
  rdb_= serializator_.get()->GetTransportCatalogue();
  renderer_ = serializator_.get()->GetRenderer();
  router_   = serializator_.get()->GetRouter();
  auto state= reader_->GetStateRequest();
  for (const auto& st : state) {
    if (st.type == "Map"){
     outvec.emplace_back(SetMapPart(st.id));
    }
    if (st.type == "Stop"){
     outvec.emplace_back(SetStopPart(st.id,st.name_));
    }
    if (st.type == "Bus"){
      outvec.emplace_back(SetBusPart(st.id,st.name_));
    }
    if (st.type == "Route"){
        outvec.emplace_back(SetRoutePart(st.id,st.from_,st.to_));
    }
  }
  //По протоколу всегда уходят массив из словарей.Их может быть много каждый раз дёргать Value не знаю как.
  reader_->PrintJSON(json::Array{outvec.begin(),outvec.end()},out);
}

json::Node RequestHandler::SetMapPart(int id) const{
  using namespace std::literals;
  auto rend = renderer_.get();
  std::ostringstream osout;
  rend->Print(osout);
  return json::Builder{}.StartDict().Key("map"s).Value(osout.str()).Key("request_id"s).Value(id).EndDict().Build();
}

json::Node RequestHandler::SetStopPart(int id,const std::string& name) const{
  using namespace std::literals;
  auto info = rdb_.get()->GetStopInfo(name);
  if (info.isFound == transport::bus_found::NOT_FOUND){
      json::Node ret = json::Builder{}.StartDict().Key("error_message"s).Value("not found"s).Key("request_id"s).Value(id).EndDict().Build();
      return  ret ;
  } else if (info.isFound == transport::bus_found::OK || info.isFound == transport::bus_found::NO_BUSES){
    json::Node ret_{json::Dict{{"buses"s, json::Array{info.buses.begin(),info.buses.end()}}, {"request_id"s, id}}};
    return ret_;
  }
  return {};
}

json::Node RequestHandler::SetBusPart(int id,const std::string& name) const{
  using namespace std::literals;
  auto  bus = rdb_.get()->GetBusStruct(name);
  if (bus != nullptr){
    int stop_count  = static_cast<int>(bus->stops.size());
    int unique_stops = static_cast<int>(bus->unique_stops);
    double leng = static_cast<double>(bus->length);
    return json::Builder{}.StartDict().Key("curvature"s).Value(bus->curvature).Key("request_id"s).Value(id)
                                    .Key("route_length"s).Value(leng).Key("stop_count"s).Value(stop_count)
                                    .Key("unique_stop_count"s).Value(unique_stops).EndDict().Build();
  } else {
    return json::Builder{}.StartDict().Key("error_message"s).Value("not found"s).Key("request_id"s).Value(id).EndDict().Build();
  }
}

json::Node RequestHandler::SetRoutePart(int id,const std::string& from,const std::string& to) const{
    if (const transport::Stop* stop_from = rdb_.get()->GetStopStruct(from)) {
        if (const transport::Stop* stop_to = rdb_.get()->GetStopStruct(to)) {
            if (auto ri = router_.get()->GetRouteInfo(stop_from, stop_to)) {
                auto [wieght, edges] = ri.value();
                return json::Node(json::Dict{
                    {{"items"s},{router_.get()->GetEdgesItems(edges)}},
                    {{"total_time"s},{wieght}},
                    {{"request_id"s},{id}}
                    });
            }
        }
    }
    return json::Builder{}.StartDict()
        .Key("error_message"s).Value("not found"s)
        .Key("request_id"s).Value(id)
        .EndDict().Build();
}

void RequestHandler::SaveDatabaseFile() {
   if (!serializator_) serializator_ = std::make_unique<serialize::Serializator>();
   serializator_.get()->Serialize(reader_);
}

RequestHandler& RequestHandler::ReadFromDB(){
    if (!serializator_) serializator_ = std::make_unique<serialize::Serializator>();
    serializator_.get()->Deserialize(reader_.get()->GetFilePath());
    return *this;
}
}//namespace requesthandler
