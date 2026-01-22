#include "PingNodePlugin.hpp"
#include "Metrics.hpp"
#include "MetricsModel.hpp"
#include "PingNodeModel.hpp"
#include "Pinger.hpp"
#include <memory>

PingNodePlugin::PrivateNode::PrivateNode(const std::unique_ptr<PingNodeModel::Node> &ptr)
    : node(*ptr), guard(*PrivateNodeCount),
      available(Metrics::Bool("PingNodeAvailible",
                              {{"name", node.get().name}, {"ip", node.get().ip}, {"url", node.get().url}})),
      latency(Metrics::Gauge("PingNodeLatency",
                             {{"name", node.get().name}, {"ip", node.get().ip}, {"url", node.get().url}}))
{
}

std::unique_ptr<Metrics::Gauge> PingNodePlugin::PrivateNode::PrivateNodeCount;

void PingNodePlugin::registerArgs(d3156::Args::Builder &bldr)
{
    bldr.setVersion("PingNodePlugin " + std::string(PingNodePlugin_VERSION));
}

void PingNodePlugin::registerModels(d3156::PluginCore::ModelsStorage &models)
{
    MetricsModel::instance()      = RegisterModel("MetricsModel", new MetricsModel(), MetricsModel);
    model                         = RegisterModel("PingNodeModel", new PingNodeModel(), PingNodeModel);
    PrivateNode::PrivateNodeCount = std::make_unique<Metrics::Gauge>("PingNodeModelNodeCount");
}

void PingNodePlugin::postInit()
{
    for (const auto &i : model->get_nodes()) nodes.push_back(std::make_unique<PrivateNode>(i));
    ping_manager =
        std::make_unique<PingManager>(nodes, model->icmp_payload, model->ping_interval_sec, model->timeout_ms);
}

PingNodePlugin::~PingNodePlugin() { 
    ping_manager.reset();
    nodes.clear();
    PrivateNode::PrivateNodeCount.reset();
}

// ABI required by d3156::PluginCore::Core (dlsym uses exact names)
extern "C" d3156::PluginCore::IPlugin *create_plugin() { return new PingNodePlugin(); }

extern "C" void destroy_plugin(d3156::PluginCore::IPlugin *p) { delete p; }

