#pragma once

#include <deque>
#include <functional>
#include <list>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>
#include <algorithm>
#include <numeric>
#include <set>
#include <string_view>
#include <unordered_set>
#include "geo.h"
#include "graph.h"
#include "router.h"
#include <iterator>

class JSONReader;
namespace transport {

struct RouteSettings {
    int bus_wait_time_ = 0;
    double bus_velocity_ = 0.0;
};
  struct Stop {
    std::string name_{};
    geo::Coordinates coordinate_{};
    Stop() = default;
    Stop(const std::string& name, double lat_, double lng_ );
    Stop(Stop&&) = default;
  };

  struct Bus {
    std::string name_{};
    int length{};
    double curvature{};
    std::vector<const Stop *> stops;
    size_t unique_stops{};
    long double route_lenth;
    bool is_circle = true;
    bool found = false;
    int req_id{};
    const Stop* final_stop;
    int pos_last_stop;
    std::vector<double>weight;
  };

  namespace bus_found {
    enum{NOT_FOUND,NO_BUSES,OK};
  }

  struct StopInfo {
    uint8_t isFound;
    std::string stop;
    std::set<std::string> buses;
  };

  struct HasherBus {
    size_t operator()(const Stop *s) const;
  };

  struct Hasher{

      size_t operator()(const std::pair<const Stop *, const Stop *>& p) const {
              return std::hash<const void*>{}(p.first) + std::hash<const void*>{}(p.second);
      }

  };

  namespace kernel {
    class TransportCatalogue {
      public:
        TransportCatalogue() = default;

        void AddStops(const std::string& stop, double lat, double lng);

        void AddBus(const std::string& name_bus, const std::vector<std::string> &stops, bool isCirle);

        const StopInfo GetStopInfo(const std::string &stop) const;

        template<typename Iterator>
        std::vector<const Bus*> GetActualBuses(Iterator begin,Iterator end) const{
            std::vector<const Bus*> tb;
            for (auto it= begin;it!=end;++it){
                tb.push_back(GetBusStruct(*it));
            }
            return tb;
        }

        template<typename Iterator>
        std::vector<const Stop *> GetActualStops(Iterator begin,Iterator end) const{
            std::vector<const Stop*> tb;
            for (auto it= begin;it!=end;++it){
                tb.push_back(GetStopStruct(*it));
            }
            return tb;
        }
        std::unordered_map<std::string_view, const Stop *> GetAllStops() const {return name_to_stop_;}
        std::unordered_map<std::string_view, const Bus *> GetAllBuses() const {return  name_to_bus_;}
        const Bus* GetBusStruct(std::string_view num_bus) const;

        const Stop* GetStopStruct(std::string_view num_bus) const;

        std::set<std::string> GetStopNBus(std::string_view stop_name) const;

        const Stop* GetNameStop(std::string_view stop) const;

        void SetDistance(const Stop* from, const Stop* to, int distance);

        int GetDistance(const Stop* from, const Stop* to) const;

      private:
        std::deque<Stop>stops_;
        std::deque<Bus> buses_;
        std::set<std::string> sort_stops;
        std::unordered_map<std::string_view, const Stop *> name_to_stop_;
        std::unordered_map<std::string_view, const Bus *> name_to_bus_;
        std::map<std::string_view, std::set<std::string>> stop_to_bus_;
        double RouteGeo(const std::vector<const Stop *> &stops) const;
        int RouteLenght(const std::vector<const Stop *> &stops) const;
        std::unordered_map<std::pair<const Stop *,const Stop *>,int,Hasher> all_names_and_dist;

        size_t GetUnique(const std::vector<const Stop *> &stops) const;




        std::pair<int,double> CalculateRoute(const std::vector<const Stop*>& stops);
    };
  }
}
