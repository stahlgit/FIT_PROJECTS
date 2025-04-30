#ifndef TCPCLIENT_HPP
#define TCPCLIENT_HPP

#include <sys/poll.h>

#include "ConnectionClient.hpp"
#include "userInput.hpp"

/**
 * @brief  Processes an incoming message received by the client.
 *
 * This function takes a `ConnectionClient` pointer, which is expected to be a `TCPClient` object,
 * and attempts to parse an incoming message using the `HandleIncomingData` method.
 * Based on the type of the parsed message (Reply, Message, Error, or Bye), it performs
 * specific actions such as displaying feedback to the user or updating the connection state.
 *
 * @param connection A pointer to the `ConnectionClient` object representing the TCP connection.
 * */
void processIncomingMessage(ConnectionClient *connection);

/**
 * @brief Handles events detected by the poll system call for the TCP client.
 *
 * This function checks the `revents` field of each `pollfd` structure to determine
 * which file descriptors have events. It then calls the appropriate handler functions
 * for incoming messages, user input, hang-up signals, and termination signals.
 *
 * @param pollFds An array of `pollfd` structures containing information about the monitored file descriptors.
 * @param connection A pointer to the `ConnectionClient` object representing the TCP connection.
 */
void handleTcpPollFds(struct pollfd *pollFds, ConnectionClient *connection);

/**
 * @brief Main loop for handling TCP communication with the server.
 *
 * This function enters a continuous loop that uses the `poll` system call to monitor
 * the connection socket, standard input, and the signal file descriptor for events.
 * When events occur, it calls the `handleTcpPollFds` function to process them.
 * The loop continues until the connection state becomes Error or End.
 *
 * @param connection A pointer to the `ConnectionClient` object representing the TCP connection.
 * @param pollFds An array of `pollfd` structures to be monitored by `poll`.
 * @param numberOfFds The number of file descriptors in the `pollFds` array.
 */
void handleTCP(ConnectionClient *connection, pollfd *pollFds, nfds_t numberOfFds);

#endif //TCPCLIENT_HPP