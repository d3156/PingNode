#include "PingNodePlugin.hpp"
#include "Pinger.hpp"
#include <PluginCore/Logger/Log>
#include <memory>
#include <ConfiguratorModel>

PingNodePlugin::PrivateNode::PrivateNode(PingNodeModel::Node &ptr)
    : node(ptr), guard(*PrivateNodeCount),
      available(Metrics::Bool("PingNodeAvailible",
                              {{"name", node.get().name}, {"ip", node.get().ip}, {"url", node.get().url}})),
      latency(Metrics::Gauge("PingNodeLatency",
                             {{"name", node.get().name}, {"ip", node.get().ip}, {"url", node.get().url}}))
{
}

std::unique_ptr<Metrics::Gauge> PingNodePlugin::PrivateNode::PrivateNodeCount;

void PingNodePlugin::registerArgs(d3156::Args::Builder &bldr) { bldr.setVersion(FULL_NAME); }

void PingNodePlugin::registerModels(d3156::PluginCore::ModelsStorage &models)
{
    MetricsModel::instance()      = models.registerModel<MetricsModel>();
    model                         = models.registerModel<PingNodeModel>();
    PrivateNode::PrivateNodeCount = std::make_unique<Metrics::Gauge>("PingNodeModelNodeCount");
    models.registerModel<ConfiguratorModel>()->registerConfig("PingNode", model->config);
    G_LOG(1, "Register module success");
}

void PingNodePlugin::postInit()
{
    for (auto &i : model->config.nodes.items) nodes.emplace_back(std::make_unique<PrivateNode>(*i));
    ping_manager = std::make_unique<PingManager>(nodes, model->config.icmp_payload, model->config.ping_interval_sec,
                                                 model->config.timeout_ms);
}

PingNodePlugin::~PingNodePlugin()
{
    ping_manager.reset();
    nodes.clear();
    PrivateNode::PrivateNodeCount.reset();
}

// ABI required by d3156::PluginCore::Core (dlsym uses exact names)
extern "C" d3156::PluginCore::IPlugin *create_plugin() { return new PingNodePlugin(); }

extern "C" void destroy_plugin(d3156::PluginCore::IPlugin *p) { delete p; }
