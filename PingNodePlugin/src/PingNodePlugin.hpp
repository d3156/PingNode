#pragma once

#include "Metrics.hpp"
#include <PluginCore/IPlugin.hpp>
#include <PluginCore/IModel.hpp>

#include <PingNodeModel.hpp>
#include <MetricsModel.hpp>
#include <vector>

#define Y_PingNode "\033[33m[PingNode]\033[0m "
#define R_PingNode "\033[31m[PingNode]\033[0m "
#define G_PingNode "\033[32m[PingNode]\033[0m "
#define W_PingNode "[PingNode] "
class PingManager;

class PingNodePlugin final : public d3156::PluginCore::IPlugin
{
    friend class Pinger;
    friend class PingManager;
    struct PrivateNode {
        PrivateNode(const std::unique_ptr<PingNodeModel::Node> &ptr);

        std::reference_wrapper<PingNodeModel::Node> node;
        Metrics::Bool available;
        Metrics::Gauge latency;
        Metrics::MetricGuard guard;
        static std::unique_ptr<Metrics::Gauge> PrivateNodeCount;
    };

    std::vector<std::unique_ptr<PrivateNode>> nodes;

    PingNodeModel *model = nullptr;
    std::unique_ptr<PingManager> ping_manager;

public:
    void registerArgs(d3156::Args::Builder &bldr) override;

    void registerModels(d3156::PluginCore::ModelsStorage &models) override;

    void postInit() override;
};
