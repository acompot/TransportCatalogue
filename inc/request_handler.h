#pragma once

#include "transport_catalogue.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "transport_router.h"
#include "serialization.h"
namespace requesthandler {

class RequestHandler {
public:
  RequestHandler () = default;
  void SendJson(std::ostream& out) ;
  void SaveDatabaseFile();
  RequestHandler& Read(std::unique_ptr<json::Document> &ptr);
  RequestHandler &ReadFromDB();
private:
   std::unique_ptr<transport::kernel::TransportCatalogue> rdb_=nullptr;
   std::unique_ptr<jsonreader::JSONReader> reader_=nullptr;
   std::unique_ptr<maprender::MapRender> renderer_=nullptr;
   std::unique_ptr<transport::Router> router_=nullptr;
   std::unique_ptr<serialize::Serializator> serializator_=nullptr;
  void Print(const json::Node& node) const ;
  json::Node SetMapPart(int id) const;
  json::Node SetStopPart(int id,const std::string& name) const;
  json::Node SetBusPart(int id,const std::string& name) const;
  json::Node SetRoutePart(int id,const std::string& from,const std::string& to) const;
};

}//namespace requesthandler
