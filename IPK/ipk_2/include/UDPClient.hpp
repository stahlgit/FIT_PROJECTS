#ifndef UDPCLIENT_HPP
#define UDPCLIENT_HPP

#include <ConnectionClient.hpp>
#include <sys/poll.h>

/**
 * @brief Calculates the timeout value for the `poll` system call based on pending messages requiring confirmation.
 *
 * If there are messages waiting for confirmation, this function calculates the time difference
 * between the current time and the scheduled retransmit timestamp of the earliest message.
 * This difference is used as the timeout for `poll`, allowing the client to retransmit messages
 * if no confirmation is received within the expected time.
 *
 * @param messagesToConfirm A constant reference to a vector of `MessageToConfirm` structures,
 * ordered by their scheduled retransmit timestamp.
 * @return The timeout value in milliseconds for `poll`. Returns -1 if there are no messages to confirm.
 */
int calculatePollTimeout(const std::vector<MessageToConfirm>& messagesToConfirm);

/**
 * @brief Handles the timeout event from the `poll` system call, retransmitting the earliest unconfirmed message.
 *
 * This function is called when the `poll` call times out, indicating that no events occurred
 * on the monitored file descriptors within the calculated timeout period. It checks if there
 * are any messages waiting for confirmation and, if so, retransmits the earliest one.
 * The retransmit timestamp and count for the message are updated accordingly.
 *
 * @param connection A pointer to the `ConnectionClient` object representing the UDP connection.
 * @param messagesToConfirm A reference to a vector of `MessageToConfirm` structures,
 * ordered by their scheduled retransmit timestamp.
 */
void handlePollTimeout(ConnectionClient* connection, std::vector<MessageToConfirm>& messagesToConfirm);
/**
 * @brief Handles an incoming UDP message received by the client.
 *
 * This function processes different types of incoming UDP messages: Confirm, Reply, Message (Msg), and Error (Err).
 * It performs actions specific to each message type, such as confirming received messages,
 * displaying replies and chat messages, and handling errors from the server.
 * For Reply, Message, and Error messages, it also sends a confirmation back to the server.
 *
 * @param connection A pointer to the `ConnectionClient` object representing the UDP connection.
 * @param message A pointer to the parsed `MessageParsing` object representing the incoming message.
 * @param messagesToConfirm A reference to a vector of `MessageToConfirm` structures,
 * used for tracking messages sent by this client that require confirmation.
 */
void handleIncomingUdpMessage(ConnectionClient* connection, MessageParsing* message, std::vector<MessageToConfirm>& messagesToConfirm);
/**
 * @brief Handles events detected by the `poll` system call for the UDP client.
 *
 * This function checks the `revents` field of each `pollfd` structure to determine
 * which file descriptors have events. It handles incoming UDP messages, user input,
 * and termination/hang-up signals. It returns an integer code to control the main loop
 * in `handleUDP`.
 *
 * @param connection A pointer to the `ConnectionClient` object representing the UDP connection.
 * @param pollFds An array of `pollfd` structures containing information about the monitored file descriptors.
 * @param messagesToConfirm A reference to a vector of `MessageToConfirm` structures.
 * @return 0 to continue the loop, 1 to break the loop, 2 to continue to the next iteration.
 */
int handleUdpPollFds(ConnectionClient* connection, struct pollfd* pollFds, std::vector<MessageToConfirm>& messagesToConfirm);
/**
 * @brief Main loop for handling UDP communication with the server.
 *
 * This function enters a continuous loop that uses the `poll` system call to monitor
 * the connection socket, standard input, and the signal file descriptor for events.
 * It calculates a dynamic timeout for `poll` based on pending messages requiring confirmation.
 * It handles timeouts (retransmitting messages) and other events by calling `handleUdpPollFds`.
 * The loop continues until the connection state becomes Error or End, and all pending
 * messages have been confirmed or the maximum number of retries has been reached.
 *
 * @param connection A pointer to the `ConnectionClient` object representing the UDP connection.
 * @param pollFds An array of `pollfd` structures to be monitored by `poll`.
 * @param numberOfFds The number of file descriptors in the `pollFds` array.
 */
void handleUDP(ConnectionClient *connection, pollfd *pollFds, nfds_t numberOfFds);

#endif //UDPCLIENT_HPP
