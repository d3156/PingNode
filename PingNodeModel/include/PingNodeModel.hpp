#pragma once
#include <PluginCore/IModel.hpp>
#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#define Y_PingNodeModel "\033[33m[PingNodeModel]\033[0m "
#define R_PingNodeModel "\033[31m[PingNodeModel]\033[0m "
#define G_PingNodeModel "\033[32m[PingNodeModel]\033[0m "
#define W_PingNodeModel "[PingNodeModel] "

class PingNodeModel final : public d3156::PluginCore::IModel
{
public:
    struct Node {
        const std::string name        = "host";
        const std::string url         = "localhost";
        const std::string ip          = "127.0.0.1";
        std::atomic<uint32_t> latency = 0;
        std::atomic<bool> available   = false;
    };

    const std::vector<std::unique_ptr<Node>> &get_nodes();

    //// Service
    d3156::PluginCore::model_name name() override { return "PingNodeModel"; }
    int deleteOrder() override { return 0; }
    void init() override {}
    void postInit() override;
    void registerArgs(d3156::Args::Builder &bldr) override;
    //// Service
private:
    std::vector<std::unique_ptr<Node>> nodes;

    std::string configPath = "./configs/PingNode.json";

    friend class PingNodePlugin;

    int ping_interval_sec    = 15;
    int timeout_ms           = 1000;
    std::string icmp_payload = "101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f3031323334353637";

    void loadSettings();
};
