#pragma once

#include <PluginCore/IPlugin.hpp>
#include <PluginCore/IModel.hpp>

#include <PingNodeModel.hpp>
#include <MetricsModel.hpp>

class PingNodePlugin final : public d3156::PluginCore::IPlugin
{

    PingNodeModel *model = nullptr;

public:
    void registerArgs(d3156::Args::Builder &bldr) override;

    void registerModels(d3156::PluginCore::ModelsStorage &models) override;

    void postInit() override;
};
