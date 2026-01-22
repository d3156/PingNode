#include "PingNodeModel.hpp"
#include <exception>
#include <filesystem>
#include <boost/json.hpp>
#include <memory>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace fs = std::filesystem;
using boost::property_tree::ptree;

std::string get_ip_from_hostname(boost::asio::io_context &io_context, const std::string &hostname)
{
    try {
        boost::asio::ip::tcp::resolver resolver(io_context);
        boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(hostname, "");
        for (const auto &endpoint : endpoints) return endpoint.endpoint().address().to_string();
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return "";
}

void PingNodeModel::loadSettings()
{

    if (!fs::exists(configPath)) {
        std::cout << Y_PingNodeModel << "Config file " << configPath << " not found. Creating default config...\n";
        fs::create_directories(fs::path(configPath).parent_path());

        ptree pt, node;
        pt.put("ping_interval_sec", ping_interval_sec);
        pt.put("timeout_ms", timeout_ms);
        pt.put("icmp_payload", icmp_payload);

        node.put("name", "host");
        node.put("url", "localhost");
        node.put("ip", "127.0.0.1");
        pt.add_child("nodes", ptree{}).push_back({"", node});

        boost::property_tree::write_json(configPath, pt);
        std::cout << G_PingNodeModel << "Default config created at " << configPath << "\n";
        return;
    }

    try {
        ptree pt;
        read_json(configPath, pt);

        ping_interval_sec = pt.get<std::uint16_t>("ping_interval_sec");
        timeout_ms        = pt.get<std::uint16_t>("timeout_ms");
        icmp_payload      = pt.get<std::string>("icmp_payload");

        const size_t MAX_ICMP_PAYLOAD_BYTES = 1472;
        if (icmp_payload.size() > MAX_ICMP_PAYLOAD_BYTES * 2)
        {
            std::cout << Y_PingNodeModel << "icmp_payload is too big. Max " << MAX_ICMP_PAYLOAD_BYTES
                      << " bytes allowed for MTU = 1500\n";
            std::cout << Y_PingNodeModel << "icmp_payload truncated to " << MAX_ICMP_PAYLOAD_BYTES << " bytes\n";
            icmp_payload = icmp_payload.substr(0, MAX_ICMP_PAYLOAD_BYTES * 2);
        }

        boost::asio::io_context io;
        for (const auto &n : pt.get_child("nodes")) {
            const auto &node = n.second;

            auto name = node.get<std::string>("name");
            auto url  = node.get<std::string>("url");
            auto ip   = node.get<std::string>("ip");

            if (ip.empty() && !url.empty()) {
                std::cout << Y_PingNodeModel << "Try to resolve ip by url " << url << "\n";
                ip = get_ip_from_hostname(io, url);
                if (!ip.empty()) std::cout << G_PingNodeModel << "Ip resolved " << ip << "\n";
            }

            if (ip.empty()) {
                std::cout << Y_PingNodeModel << "Ip is empty, node " << name << " skipped\n";
                continue;
            }

            nodes.push_back(std::make_unique<Node>(name, url, ip, 0, false));
        }
    } catch (std::exception e) {
        std::cout << R_PingNodeModel << "error on load config " << configPath << " " << e.what() << std::endl;
    }
}

void PingNodeModel::registerArgs(d3156::Args::Builder &bldr)
{
    bldr.setVersion("PingNodeModel " + std::string(PingNodeModel_VERSION))
        .addOption(configPath, "PingNodeConfig", "Path to PingNode.json");
}

void PingNodeModel::postInit() { loadSettings(); }

const std::vector<std::unique_ptr<PingNodeModel::Node>> &PingNodeModel::get_nodes() { return nodes; }
