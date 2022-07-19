#pragma once

#include "json.h"
#include "json_builder.h"
#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"
#include "json_reader.h"

#include <optional>
#include <string>
#include <string_view>
class JSONReader;
namespace transport {


    class Router {
    public:
        Router() = default;

        void SetSettings(const RouteSettings& settings);

        void BuildGraph(const std::unique_ptr<transport::kernel::TransportCatalogue>& ptc
                        ,const std::vector<const transport::Stop*>& actual_stops
                        ,const std::vector<const transport::Bus*>& all_buses);

        json::Array GetEdgesItems(const std::vector<graph::EdgeId>& edges) const;

        std::optional<graph::Router<double>::RouteInfo> GetRouteInfo(const Stop* from, const Stop* to) const;

        size_t GetGraphVertexCount();

        const std::map<std::string, graph::VertexId>& GetStopIds() const;

        const graph::DirectedWeightedGraph<double>& GetGraph() const;

        json::Node GetSettings() const;

        void SetGraph(graph::DirectedWeightedGraph<double>&& graph,
              std::map<std::string, graph::VertexId>&& stop_ids);
    private:
        RouteSettings settings_;
        graph::DirectedWeightedGraph<double> graph_;
        std::map<std::string, graph::VertexId> stop_ids_;
        std::unique_ptr<graph::Router<double>> ptr_router_ = nullptr;
        constexpr static double KOEF_TO_METER_BY_MINUTE = 1000.0/ 60.0;
        void SetSettings(const json::Node& settings_node);
    };

} // namespace transport
