#pragma once

#include <PluginCore/IPlugin>
#include <PluginCore/IModel>

#include <PingNodeModel>
#include <MetricsModel/MetricsModel>
#include <vector>

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

    ~PingNodePlugin();

public:
    void registerArgs(d3156::Args::Builder &bldr) override;

    void registerModels(d3156::PluginCore::ModelsStorage &models) override;

    void postInit() override;
};
