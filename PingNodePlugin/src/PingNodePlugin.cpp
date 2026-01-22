#include "PingNodePlugin.hpp"

// ABI required by d3156::PluginCore::Core (dlsym uses exact names)
extern "C" d3156::PluginCore::IPlugin *create_plugin() { return new PingNodePlugin(); }

extern "C" void destroy_plugin(d3156::PluginCore::IPlugin *p) { delete p; }

void PingNodePlugin::registerArgs(d3156::Args::Builder &bldr)
{
    bldr.setVersion("PingNodePlugin " + std::string(PingNodePlugin_VERSION));
}

void PingNodePlugin::registerModels(d3156::PluginCore::ModelsStorage &models)
{
    MetricsModel::instance() = RegisterModel("MetricsModel", new MetricsModel(), MetricsModel);
    model                    = RegisterModel("PingNodeModel", new PingNodeModel(), PingNodeModel);
}

void PingNodePlugin::postInit() {}
