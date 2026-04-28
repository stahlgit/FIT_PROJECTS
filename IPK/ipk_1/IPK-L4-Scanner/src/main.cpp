/**
 * @file main.cpp
 * @brief Main entry point for the application
 * @author Peter Stahl (xstahl01)
 */
#include "application.hpp"
#include <csignal>

// Global application instance - allows signal handlers to access the application for graceful shutdown
static Application app;

// Signal handler for graceful shutdown on SIGINT and SIGTERM
static void signal_handler(int signum) {
    (void)signum; 
    app.shutdown();
}

/** @brief Main entry point for the application */
int main(int argc, char* argv[]) {
    std::signal(SIGINT,  signal_handler);
    std::signal(SIGTERM, signal_handler);
    return app.run(argc, argv);
}