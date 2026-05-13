/**
 * @file app_factory.hpp
 * @brief Factory that constructs the correct App subclass from a ConnConfig.
 * @author Peter Stahl (xstahl01)
 */

#pragma once
#ifndef APP_FACTORY_HPP
#define APP_FACTORY_HPP

#include "app.hpp"

#include <memory>

namespace rdt::app
{

    class AppFactory
    {
    public:
        ///@brief Create a ClientApp or ServerApp depending on config.mode (Assigns a random conn_id for CLIENT mode)
        [[nodiscard]] static std::unique_ptr<App> create(ConnConfig config);

    private:
        /// @brief Generate Connection ID 
        [[nodiscard]] static uint32_t generate_conn_id_();
    };

} // namespace rdt::app

#endif // APP_FACTORY_HPP
