#pragma once
#include "PingNodePlugin.hpp"
#include <string>

using boost::asio::ip::icmp;

std::vector<unsigned char> hex_to_binary(const std::string &hex);

class Pinger : public std::enable_shared_from_this<Pinger>
{
public:
    Pinger(boost::asio::io_context &io, PingNodePlugin::PrivateNode &pnode, const std::vector<unsigned char> &payload,
           int timeout_ms);

    void start();

    void cancel(const std::string &err = {});

private:
    void send_echo();

    void start_receive();

    PingNodePlugin::PrivateNode &pnode_;
    icmp::socket socket_;
    icmp::endpoint dest_, sender_;
    boost::asio::steady_timer timer_;
    icmp::resolver resolver_;
    std::vector<unsigned char> payload_, reply_;
    std::chrono::steady_clock::time_point start_time_;
    int timeout_ms_;
    uint16_t this_sequence_number;
};

class PingManager
{
public:
    PingManager(std::vector<std::unique_ptr<PingNodePlugin::PrivateNode>> &nodes, const std::string &icmp_payload,
                int ping_interval_sec, int timeout_ms);

    ~PingManager();

private:
    void schedule_ping();

    std::vector<std::unique_ptr<PingNodePlugin::PrivateNode>> &nodes_;
    boost::asio::io_context io_;
    boost::asio::steady_timer timer_;
    std::vector<unsigned char> payload_;
    std::vector<std::shared_ptr<Pinger>> active_pingers_;
    int ping_interval_sec_;
    int timeout_ms_;
    boost::thread thread_;
};