/**
 * @file app_factory.cpp
 * @brief AppFactory implementation.
 * @author Peter Stahl (xstahl01)
 */

#include "app_factory.hpp"
#include "client_app.hpp"
#include "server_app.hpp"

#include <limits>
#include <random>

namespace rdt::app
{

uint32_t AppFactory::generate_conn_id_()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dist(1, std::numeric_limits<uint32_t>::max());
    return dist(gen);
}

std::unique_ptr<App> AppFactory::create(ConnConfig config)
{
    if (config.mode == rdt::AppMode::CLIENT) {
        config.conn_id = generate_conn_id_();
        return std::make_unique<ClientApp>(std::move(config));
    }
    return std::make_unique<ServerApp>(std::move(config));
}

} // namespace rdt::app
