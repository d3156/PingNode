#include "Pinger.hpp"
#include "PingNodePlugin.hpp"
#include <ostream>
#include <string>

std::vector<unsigned char> hex_to_binary(const std::string &hex)
{
    std::vector<unsigned char> bin;
    for (size_t i = 0; i < hex.length(); i += 2)
        bin.push_back(static_cast<unsigned char>(std::stoul(hex.substr(i, 2), nullptr, 16)));
    return bin;
}

struct IcmpHeader {
    uint8_t type;
    uint8_t code;
    uint16_t checksum;
    uint16_t identifier;
    uint16_t sequence_number;
};

uint16_t icmp_checksum(const void *data, size_t length)
{
    const uint16_t *ptr = reinterpret_cast<const uint16_t *>(data);
    uint32_t sum        = 0;
    while (length > 1) {
        sum += *ptr++;
        length -= 2;
    }
    if (length == 1) { sum += *reinterpret_cast<const uint8_t *>(ptr); }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return static_cast<uint16_t>(~sum);
}

std::vector<unsigned char> make_icmp_packet(std::vector<unsigned char> payload, uint16_t identifier,
                                            uint16_t sequence_number)
{
    IcmpHeader hdr{8, 0, 0, htons(identifier), htons(sequence_number)};
    std::vector<unsigned char> packet(sizeof(IcmpHeader) + payload.size());
    std::memcpy(packet.data(), &hdr, sizeof(IcmpHeader));
    std::memcpy(packet.data() + sizeof(IcmpHeader), payload.data(), payload.size());
    IcmpHeader *hdr_ptr = reinterpret_cast<IcmpHeader *>(packet.data());
    hdr_ptr->checksum   = icmp_checksum(packet.data(), packet.size());
    return packet;
}

Pinger::Pinger(boost::asio::io_context &io, PingNodePlugin::PrivateNode &pnode,
               const std::vector<unsigned char> &payload, int timeout_ms)
    : pnode_(pnode), socket_(io, icmp::v4()), timer_(io), resolver_(io), payload_(payload), timeout_ms_(timeout_ms)
{
}

void Pinger::start()
{
    auto self = shared_from_this();
    try {
        self->dest_ = icmp::endpoint(boost::asio::ip::make_address(self->pnode_.node.get().ip), 0);
    } catch (const std::exception &e) {
        self->cancel(std::string("Fail to parse IP: ") + e.what());
        return;
    }
    self->send_echo();
    timer_.expires_after(std::chrono::milliseconds(timeout_ms_));
    timer_.async_wait([self](const boost::system::error_code &ec) {
        if (!ec) self->cancel();
    });
}

void Pinger::cancel(const std::string &err)
{
    if (err.size()) std::cout << R_PingNode << err << std::endl;
    timer_.cancel();
    if (socket_.is_open()) socket_.cancel();
    pnode_.latency              = 0;
    pnode_.available            = false;
    pnode_.node.get().latency   = 0;
    pnode_.node.get().available = false;
}

void Pinger::send_echo()
{
    start_time_                       = std::chrono::steady_clock::now();
    auto self                         = shared_from_this();
    static uint16_t identifier        = static_cast<uint16_t>(::getpid() & 0xFFFF);
    static uint16_t sequence_number   = 0;
    this_sequence_number              = ++sequence_number;
    std::vector<unsigned char> packet = make_icmp_packet(self->payload_, identifier, this_sequence_number);
    socket_.async_send_to(boost::asio::buffer(packet), dest_, [self](const boost::system::error_code &ec, size_t) {
        if (ec) self->cancel("Fail to send ICMP packet with error " + ec.message());
    });
    reply_.resize(1500);
    start_receive();
}

void Pinger::start_receive()
{
    auto self          = shared_from_this();
    auto recv_callback = [self](const boost::system::error_code &ec, size_t len) {
        if (ec) {
            if (ec != boost::asio::error::operation_aborted)
                self->cancel("Fail to recv ICMP reply with error " + ec.message());
            return;
        }
        size_t ip_header_len = (self->reply_[0] & 0x0F) * 4;
        if (len < ip_header_len + sizeof(IcmpHeader)) {
            self->start_receive();
            return;
        }
        IcmpHeader *hdr            = reinterpret_cast<IcmpHeader *>(self->reply_.data() + ip_header_len);
        static uint16_t identifier = static_cast<uint16_t>(::getpid() & 0xFFFF);
        if (hdr->type != 0 || hdr->code != 0 || ntohs(hdr->identifier) != identifier ||
            ntohs(hdr->sequence_number) != self->this_sequence_number) {
            self->start_receive();
            return;
        }
        auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - self->start_time_);
        self->timer_.cancel();
        self->pnode_.latency              = static_cast<uint32_t>(elapsed.count());
        self->pnode_.available            = true;
        self->pnode_.node.get().latency   = static_cast<uint32_t>(elapsed.count());
        self->pnode_.node.get().available = true;
        std::cout << self->pnode_.node.get().name << " latency: " << elapsed.count() << " ms" << std::endl;
    };
    socket_.async_receive_from(boost::asio::buffer(reply_), sender_, std::move(recv_callback));
}

PingManager::PingManager(std::vector<std::unique_ptr<PingNodePlugin::PrivateNode>> &nodes,
                         const std::string &icmp_payload, int ping_interval_sec, int timeout_ms)
    : nodes_(nodes), io_(), timer_(io_), payload_(hex_to_binary(icmp_payload)), ping_interval_sec_(ping_interval_sec),
      timeout_ms_(timeout_ms), thread_([this]() {
          schedule_ping();
          std::cout << G_PingNode << "PingManager run" << std::endl;
          io_.run();
      })
{
}

PingManager::~PingManager()
{
    try {
        timer_.cancel();
        for (auto &p : active_pingers_) p->cancel();
        active_pingers_.clear();
        if (!thread_.joinable()) return;
        int stopThreadTimeoutMs = 200;
        std::cout << G_PingNode << "Trying to join PingManager thread for " << stopThreadTimeoutMs << "ms\n";
        if (thread_.timed_join(boost::posix_time::milliseconds(stopThreadTimeoutMs))) {
            std::cout << G_PingNode << "PingManager thread joined successfully" << std::endl;
            return;
        }
        std::cout << Y_PingNode << "PingManager thread did not terminate in time, forcing io_context stop...\n";
        io_.stop();
        if (thread_.timed_join(stopThreadTimeoutMs)) {
            std::cout << G_PingNode << "PingManager thread force-stopped successfully" << std::endl;
            return;
        }
        std::cout << R_PingNode
                  << "WARNING: PingManager thread cannot be stopped. Detaching thread (potential resource leak)\n";
        thread_.detach();
    } catch (const std::exception &e) {
        std::cout << R_PingNode << "Exception in PingManager destructor: " << e.what() << std::endl;
    }
}

void PingManager::schedule_ping()
{
    for (auto &p : active_pingers_) p->cancel();
    active_pingers_.clear();
    for (auto &n : nodes_) {
        try {
            auto pinger = std::make_shared<Pinger>(io_, *n, payload_, timeout_ms_);
            pinger->start();
            active_pingers_.push_back(pinger);
        } catch (const std::exception &e) {
            std::cout << R_PingNode << "Error pinging " << n->node.get().name << ": " << e.what() << "\n";
        }
    }
    timer_.expires_after(std::chrono::seconds(ping_interval_sec_));
    timer_.async_wait([this](const boost::system::error_code &ec) {
        if (!ec) schedule_ping();
    });
}
