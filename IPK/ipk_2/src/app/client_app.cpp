/**
 * @file client_app.cpp
 * @brief ClientApp::run - wires infra into SenderSession and executes the transfer.
 * @author Peter Stahl (xstahl01)
 */

#include "client_app.hpp"
#include "signal_handler.hpp"
#include "dns_resolver.hpp"
#include "udp_socket.hpp"
#include "stream_io.hpp"
#include "sender_session.hpp"
#include "logger.hpp"
#include "exceptions.hpp"

#include <string>

namespace rdt::app
{

ClientApp::ClientApp(ConnConfig config) : App(std::move(config)) {}

rdt::ExitCode ClientApp::run()
{
    auto const &log = rdt::utils::Logger::instance();
    rdt::infra::SignalHandler sig(stop_);

    try {
        auto addr = rdt::infra::DnsResolver::resolve_first(config_.host, config_.port);
        if (!addr)
            throw rdt::DnsException("could not resolve host: " + config_.host);

        log.info("client", "resolved {}", config_.host);

        auto sock = rdt::infra::UdpSocket::connect_client(addr->addr, addr->len);
        auto stream = rdt::infra::StreamIO::open_input(config_.input_path);

        rdt::domain::SenderSession session(config_, sock.fd(), stop_, stream.input());
        auto result = session.run();

        if (stop_.load()) {
            log.warn("client", "transfer interrupted by signal");
            return rdt::ExitCode::ERR_INTERNAL;
        }

        if (result == rdt::TransferResult::SUCCESS) {
            log.info("client", "transfer complete");
            return rdt::ExitCode::OK;
        }

        if (result == rdt::TransferResult::TIMEOUT) {
            log.error("client", "transfer timed out (no progress for {}s)", config_.timeout_s);
            return rdt::ExitCode::ERR_TIMEOUT;
        }

        log.error("client", "transfer failed");
        return rdt::ExitCode::ERR_PROTOCOL;

    } catch (const rdt::RdtException &e) {
        log.error("client", e.what());
        return e.exit_code();
    }
}

} // namespace rdt::app
