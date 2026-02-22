#include "PingNodeModel.hpp"
#include <exception>
#include <boost/asio.hpp>
#include <PluginCore/Logger/Log>

std::string get_ip_from_hostname(boost::asio::io_context &io_context, const std::string &hostname)
{
    try {
        boost::asio::ip::tcp::resolver resolver(io_context);
        boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(hostname, "");
        for (const auto &endpoint : endpoints) return endpoint.endpoint().address().to_string();
    } catch (const std::exception &e) {
        R_LOG(1, "Error: " << e.what());
    }
    return "";
}

void PingNodeModel::registerArgs(d3156::Args::Builder &bldr) { bldr.setVersion(FULL_NAME); }

std::vector<std::unique_ptr<PingNodeModel::Node>> &PingNodeModel::get_nodes() { return config.nodes.items; }

std::string PingNodeModel::name() { return FULL_NAME; }

PingNodeModel::Node::Node(Node &&other)
    : d3156::Config(""), name(std::move(other.name)), url(std::move(other.url)), ip(std::move(other.ip)),
      latency(other.latency.load(std::memory_order_relaxed)), available(other.available.load(std::memory_order_relaxed))
{
}

PingNodeModel::Node::Node() : d3156::Config("") {}

void PingNodeModel::postInit()
{
    boost::asio::io_context io;
    for (auto &i : config.nodes.items)
        if (i->ip.value.empty() && !i->url.value.empty()) i->ip.value = get_ip_from_hostname(io, i->url.value);
}
