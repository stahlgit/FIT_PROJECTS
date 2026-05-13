/**
 * @file server_app.cpp
 * @brief ServerApp::run - wires infra into ReceiverSession and executes the transfer.
 * @author Peter Stahl (xstahl01)
 */

#include "server_app.hpp"
#include "signal_handler.hpp"
#include "udp_socket.hpp"
#include "stream_io.hpp"
#include "receiver_session.hpp"
#include "logger.hpp"
#include "exceptions.hpp"

#include <string>
#include <format>

namespace rdt::app
{

ServerApp::ServerApp(ConnConfig config) : App(std::move(config)) {}

rdt::ExitCode ServerApp::run()
{
    auto const &log = rdt::utils::Logger::instance();
    rdt::infra::SignalHandler sig(stop_);

    try {
        auto sock = rdt::infra::UdpSocket::bind_server(config_.bind_adress, config_.port);
        auto stream = rdt::infra::StreamIO::open_output(config_.output_path);

        log.info("server", "listening on port {}", config_.port);

        rdt::domain::ReceiverSession session(config_, sock.fd(), stop_, stream.output());
        auto result = session.run();

        if (stop_.load()) {
            log.warn("server", "transfer interrupted by signal");
            return rdt::ExitCode::ERR_INTERNAL;
        }

        if (result == rdt::TransferResult::SUCCESS) {
            log.info("server", "transfer complete");
            return rdt::ExitCode::OK;
        }

        if (result == rdt::TransferResult::TIMEOUT) {
            log.error("server", "transfer timed out (no progress for {}s)", config_.timeout_s);
            return rdt::ExitCode::ERR_TIMEOUT;
        }

        log.error("server", "transfer failed");
        return rdt::ExitCode::ERR_PROTOCOL;

    } catch (const rdt::RdtException &e) {
        log.error("server", e.what());
        return e.exit_code();
    }
}

} // namespace rdt::app
