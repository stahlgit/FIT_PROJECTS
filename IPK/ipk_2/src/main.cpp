/**
 * @file main.cpp
 * @brief Entry point - parse arguments, build the app, run.
 * @author Peter Stahl (xstahl01)
 */

#include "arg_parser.hpp"
#include "app_factory.hpp"
#include "exceptions.hpp"

#include <iostream>

int main(int argc, char *argv[])
{
    try {
        auto config = rdt::app::ArgParser::parse(argc, argv);
        auto app = rdt::app::AppFactory::create(std::move(config));
        return static_cast<int>(app->run());
    } catch (const rdt::ArgException &e) {
        std::cerr << "Error: " << e.what() << "\n";
        return static_cast<int>(rdt::ExitCode::ERR_ARGS);
    } catch (const std::exception &e) {
        std::cerr << "Unexpected error: " << e.what() << "\n";
        return static_cast<int>(rdt::ExitCode::ERR_INTERNAL);
    }
}
