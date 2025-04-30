#include "ArgumentParser.hpp"
#include <iostream>
#include <sys/poll.h>
#include <cstring>
#include <sys/signalfd.h>
#include <csignal>
#include "ConnectionClient.hpp"
#include "userInput.hpp"
#include "UDPClient.hpp"
#include "TCPClient.hpp"

using namespace std;
int signalFd;

/**
 * @brief Main function for ipk25chat-client
 *
 * This function parses command-line arguments, establishes a connection
 * to the server (either TCP or UDP), sets up signal handling for graceful
 * termination, and then enters the main event loop to handle communication
 * with the server and user input.
 *
 * @param argc The number of command-line arguments.
 * @param argv An array of C-style strings containing the command-line arguments.
 * @return EXIT_SUCCESS if the application runs successfully, EXIT_FAILURE otherwise.
 */
int main(int argc, char* argv[]) {
    try {
        ArgumentParser parser(argc, argv);
        ProgramOptions options = parser.parse();

        ConnectionClient *connection;
        // Determine the transport protocol based on the parsed options.
        if (options.transport == "tcp"){
            connection = createTCPConnection(options.server, options.port);
        }
        else if (options.transport == "udp"){
            connection = createUDPConnection(options.server, options.port, options.udpTimeoutMs, options.maxRetries);
        }
        else{
            throw invalid_argument("Unknown transport type");
        }

        // Set up signal handling to catch SIGINT (Ctrl+C).
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGINT);
        sigprocmask(SIG_BLOCK, &mask, nullptr);

        // Create a signal file descriptor to monitor for blocked signals.
        int sigfd = signalfd(-1, &mask, 0); // create signalfd for poll

        // Define the file descriptors to be monitored by the poll system call.
        struct pollfd socketPollFd = {.fd = connection->GetSocket(), .events = POLLIN}; // Monitor the connection socket for incoming data.
        struct pollfd stdinPollFd = {.fd = 0 , .events = POLLIN | POLLHUP}; // Monitor standard input for user input and hang-up events.
        struct pollfd signalPollFd = {.fd = sigfd, .events = POLLIN | POLLERR | POLLHUP}; // Monitor the signal file descriptor for caught signals, errors and hang-ups
        struct pollfd pollFds[] = {socketPollFd, stdinPollFd, signalPollFd}; // Array of pollfd structures to be passed to poll.
        // number of file descriptors to monitor.
        nfds_t numberOfFds = 3;

        if (options.transport == "tcp")
        {
            handleTCP(connection, pollFds, numberOfFds);
        }
        else if (options.transport == "udp")
        {
            handleUDP(connection, pollFds, numberOfFds);
        }
        else
        {
            throw runtime_error("Unknown transport type");
        }

    } catch (const exception& e) {
        cerr << "ERROR: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
