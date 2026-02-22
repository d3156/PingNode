#pragma once
#include <PluginCore/IModel>
#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <BaseConfig>

class PingNodeModel final : public d3156::PluginCore::IModel
{
public:
    struct Node : public d3156::Config {
        Node();
        Node(Node &&other);
        CONFIG_STRING(name, "host");
        CONFIG_STRING(url, "localhost");
        CONFIG_STRING(ip, "127.0.0.1");
        std::atomic<uint32_t> latency = 0;
        std::atomic<bool> available   = false;
    };

    struct Config : public d3156::Config {
        Config() : d3156::Config("") {}
        CONFIG_UINT(ping_interval_sec, 15);
        CONFIG_UINT(timeout_ms, 1000);
        CONFIG_STRING(icmp_payload, "101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f3031323334353637");
        CONFIG_ARRAY(nodes, Node);
    };

    std::vector<std::unique_ptr<Node>> &get_nodes();

    //// Service
    static std::string name();
    int deleteOrder() override { return 0; }
    void init() override {}
    void registerArgs(d3156::Args::Builder &bldr) override;
    void postInit() override;
    //// Service
private:
    Config config;
    friend class PingNodePlugin;
};
