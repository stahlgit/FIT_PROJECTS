/**
 * @file app.hpp
 * @brief Abstract base class for ClientApp and ServerApp.
 * @author Peter Stahl (xstahl01)
 */

#pragma once
#ifndef APP_HPP
#define APP_HPP

#include "connconfig.hpp"
#include "rdt_constants.hpp"

#include <atomic>

namespace rdt::app
{

    class App
    {
    public:
        virtual ~App() = default;

        App(const App &) = delete;
        App &operator=(const App &) = delete;
        App(App &&) = delete;
        App &operator=(App &&) = delete;

        [[nodiscard]] virtual rdt::ExitCode run() = 0;

    protected:
        explicit App(ConnConfig config) : config_(std::move(config)) {}

        ConnConfig config_;
        std::atomic<bool> stop_{false};
    };

} // namespace rdt::app

#endif // APP_HPP
