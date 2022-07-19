#include "transport_router.h"

#include <string>
#include <string_view>

#include <map>
#include <utility>
#include <vector>
#include <algorithm>

using namespace std;

namespace transport {

    void Router::SetSettings(const RouteSettings& settings) {
        settings_ = settings;
    }



    void Router::BuildGraph(const std::unique_ptr<transport::kernel::TransportCatalogue>& ptc
                            ,const std::vector<const transport::Stop*>& actual_stops
                            ,const std::vector<const Bus *> &all_buses)
    {

          auto tc =ptc.get();

        graph::DirectedWeightedGraph<double> stops_graph(actual_stops.size() * 2);
        map<std::string, graph::VertexId> stop_ids;
        graph::VertexId vertex_id = 0;
        double wait_time =  static_cast<double>(settings_.bus_wait_time_);
        for (const auto& stop_ptr : actual_stops) {
            stop_ids[stop_ptr->name_] = vertex_id;
            stops_graph.AddEdge({ stop_ptr->name_,
                                  0,
                                  vertex_id,
                                  ++vertex_id,
                                  wait_time });
            ++vertex_id;
        }
        stop_ids_ = move(stop_ids);


        for_each(
            all_buses.begin(),
            all_buses.end(),
            [&stops_graph,&tc, this](const auto& bus_ptr)
            {
                const std::vector<const Stop*> stops = bus_ptr->stops;
                size_t stops_count = stops.size();
                for (size_t i = 0; i < stops_count; ++i) {
                    for (size_t j = i + 1; j < stops_count; ++j) {
                        const Stop* stop_from = stops[i];
                        const Stop* stop_to = stops[j];
                        int dist_sum = 0;
                        for (size_t k = i + 1; k <= j; ++k) {
                            dist_sum += tc->GetDistance(stops[k - 1],stops[k]);
                        }
                        stops_graph.AddEdge({ bus_ptr->name_,
                                              j - i,
                                              stop_ids_.at(stop_from->name_) + 1,
                                              stop_ids_.at(stop_to->name_),
                                              static_cast<double>(dist_sum) / (settings_.bus_velocity_ * (KOEF_TO_METER_BY_MINUTE)) });
                        if (!bus_ptr->is_circle && stop_to == bus_ptr->final_stop && j == stops_count / 2) break;
                    }
                }


            });

         graph_ = move(stops_graph);
         ptr_router_ =(std::make_unique<graph::Router<double>>(graph_));


    }

    json::Array Router::GetEdgesItems(const std::vector<graph::EdgeId>& edges) const
    {
        json::Array items_array;
        items_array.reserve(edges.size());
        for (auto& edge_id : edges) {
            const graph::Edge<double>& edge = graph_.GetEdge(edge_id);
            if (edge.quality == 0) {
                items_array.emplace_back(json::Node(json::Dict{
                    {{"type"s},{"Wait"s}},
                    {{"stop_name"s},{static_cast<string>(edge.name)}},
                    {{"time"s},{edge.weight}}
                    }));
            }
            else {
                items_array.emplace_back(json::Node(json::Dict{
                     {{"type"s},{"Bus"s}},
                    {{"bus"s},{static_cast<string>(edge.name)}},
                    {{"span_count"s},{static_cast<int>(edge.quality)}},
                    {{"time"s},{edge.weight}}
                    }));
            }
        }
        return items_array;
    }

    std::optional<graph::Router<double>::RouteInfo> Router::GetRouteInfo(const Stop* from, const Stop* to) const
    {
        return ptr_router_.get()->BuildRoute(stop_ids_.at(from->name_), stop_ids_.at(to->name_));
    }

    size_t Router::GetGraphVertexCount()
    {
        return graph_.GetVertexCount();
    }

    const std::map<std::string, graph::VertexId>& Router::GetStopIds() const {
        return stop_ids_;
    }

    const graph::DirectedWeightedGraph<double>& Router::GetGraph() const {
        return graph_;
    }

    void Router::SetGraph(graph::DirectedWeightedGraph<double>&& graph
                         ,std::map<std::string, graph::VertexId>&& stop_ids)
    {
          graph_ = move(graph);
          stop_ids_ = move(stop_ids);
          ptr_router_ =(std::make_unique<graph::Router<double>>(graph_));
      }




} // transport
