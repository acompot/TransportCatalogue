#include "transport_catalogue.h"
#include "geo.h"


namespace transport {

  Stop::Stop(const std::string& name, double lat_, double lng_ ){
    name_ = std::move(name);
    coordinate_.lat = lat_;
    coordinate_.lng = lng_;
  }

  size_t HasherBus::operator()(const Stop *s) const {
    static constexpr int X = 17;
    return  std::hash<std::string> {}(s->name_) * X * X * X
            + std::hash<double> {}(s->coordinate_.lat) * X  * X
            + std::hash<double> {}(s->coordinate_.lng);
  }



  namespace kernel {

    void TransportCatalogue::AddStops(const std::string &stop,double lat, double lng) {
      if (name_to_stop_.count(stop) < 1) {
//      all_names_and_dist= std::move(rasst);
      stops_.emplace_back(std::string(stop),lat,lng);
      name_to_stop_.emplace(stops_.back().name_, &stops_.back());
      stop_to_bus_[stops_.back().name_] = {};

      }
    }

    void TransportCatalogue::AddBus( const std::string& name_bus,
                                     const std::vector<std::string> &stops, bool isCirle) {
      if (name_bus.empty() || stops.empty()) return;
      if (name_to_bus_.count(name_bus) < 1) {
        buses_.push_back({});
        Bus& ref_last_bus = buses_.back();
        ref_last_bus.name_ = name_bus;
        for (auto name_stop : stops) {
          ref_last_bus.stops.push_back(name_to_stop_.at(name_stop));
          stop_to_bus_[name_stop].insert(name_bus);
        }
        if (isCirle == false){
          ref_last_bus.pos_last_stop = ref_last_bus.stops.size()-1;
          for (auto it =stops.rbegin() +1; it!=stops.rend();++it){
            ref_last_bus.stops.push_back(name_to_stop_.at(*(it)));
          }
          ref_last_bus.final_stop = name_to_stop_.at(*(stops.rend() -1));
        }
        auto [length,curvature]  =  CalculateRoute(ref_last_bus.stops);
        ref_last_bus.length = length;
        ref_last_bus.curvature = curvature;
        ref_last_bus.route_lenth= RouteGeo(ref_last_bus.stops);
        ref_last_bus.found = true;
        ref_last_bus.is_circle = isCirle;
        ref_last_bus.unique_stops = GetUnique(ref_last_bus.stops);
        name_to_bus_.emplace(ref_last_bus.name_, &ref_last_bus);
      }
    }

    std::pair<int,double> TransportCatalogue::CalculateRoute(const std::vector<const Stop*>& stops){
      int length = RouteLenght(stops);
      double coord_length = RouteGeo(stops);

      coord_length = static_cast<double>(length / std::abs(coord_length));

      return {length,coord_length};
    }

    const Bus* TransportCatalogue::GetBusStruct(std::string_view num_bus) const {
      return (name_to_bus_.count(num_bus) > 0) ? name_to_bus_.at(num_bus) : nullptr;
    }

    const Stop* TransportCatalogue::GetStopStruct(std::string_view stop) const {
      return (name_to_stop_.count(stop) > 0) ? name_to_stop_.at(stop) : nullptr;
    }

    size_t TransportCatalogue::GetUnique(const std::vector<const Stop*>& stops) const {
      std::unordered_set<const Stop*, HasherBus> unique(stops.begin(), stops.end());
      return unique.size();
    }

    double TransportCatalogue::RouteGeo(const std::vector<const Stop*>& stops) const {
      auto lyambda_distance_pair_coord = [](const Stop* a, const Stop* b) {
        return geo::ComputeDistance(a->coordinate_, b->coordinate_);
      };
      return std::transform_reduce( std::next(stops.begin( )),
                                    stops.end( ), stops.begin( ), 0.0, std::plus<>( ),
                                    lyambda_distance_pair_coord);
    }

    int TransportCatalogue::RouteLenght(const std::vector<const Stop *> &stops) const{
      auto lyambda_distance_pair_int = [this](const Stop* a, const Stop* b) {
        if (all_names_and_dist.count(std::make_pair(b,a))) return all_names_and_dist.at(std::make_pair(b,a));
        else  return all_names_and_dist.at(std::make_pair(a,b));
      };
      return std::transform_reduce( std::next(stops.begin( )),
                                    stops.end( ), stops.begin( ), 0, std::plus<>( ),
                                    lyambda_distance_pair_int);
    }

    const StopInfo TransportCatalogue::GetStopInfo(const std::string& stop) const {
      StopInfo info;
      if (stop_to_bus_.count(stop) > 0){
        std::set<std::string> sorted_buses = stop_to_bus_.at(stop);
        if (sorted_buses.empty()) info = StopInfo{transport::bus_found::NO_BUSES,{},{}};
        else  info= StopInfo{transport::bus_found::OK,stop,sorted_buses};
      }
      else info= StopInfo{transport::bus_found::NOT_FOUND,{},{}};
      return info;
    }

    std::set<std::string> TransportCatalogue::GetStopNBus(std::string_view stop_name) const{
        return stop_to_bus_.at(stop_name);
    }

    void TransportCatalogue::SetDistance(const Stop* from,const Stop* to,int distance){
        all_names_and_dist[std::make_pair(from,to)] = distance;
    }

    int TransportCatalogue::GetDistance(const Stop* from,const Stop* to) const{
        if (all_names_and_dist.count(std::make_pair(from,to))) return all_names_and_dist.at(std::make_pair(from,to));
        else if (all_names_and_dist.count(std::make_pair(to,from))) return all_names_and_dist.at(std::make_pair(to,from));
        else return 0;
    }

  }
}
