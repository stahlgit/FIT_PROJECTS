/**
 * @file client_app.hpp
 * @brief Sending side of the application (client mode).
 * @author Peter Stahl (xstahl01)
 */

#pragma once
#ifndef CLIENT_APP_HPP
#define CLIENT_APP_HPP

#include "app.hpp"

namespace rdt::app
{

    class ClientApp final : public App
    {
    public:
        explicit ClientApp(ConnConfig config);

        /// @brief Main Entrypoint for Client App
        [[nodiscard]] rdt::ExitCode run() override;
    };

} // namespace rdt::app

#endif // CLIENT_APP_HPP
