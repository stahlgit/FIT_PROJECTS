/**
 * @file server_app.hpp
 * @brief Receiving side of the application (server mode).
 * @author Peter Stahl (xstahl01)
 */

#pragma once
#ifndef SERVER_APP_HPP
#define SERVER_APP_HPP

#include "app.hpp"

namespace rdt::app
{

    class ServerApp final : public App
    {
    public:
        explicit ServerApp(ConnConfig config);

        /// @brief main entry point for Server App
        [[nodiscard]] rdt::ExitCode run() override;
    };

} // namespace rdt::app

#endif // SERVER_APP_HPP
