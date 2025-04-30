#ifndef USER_INPUT_HPP
#define USER_INPUT_HPP

#include <regex>
#include "ConnectionClient.hpp"

/**
 * @brief Handles user input from standard input.
 *
 * This function reads a line of input from the user, attempts to match it against
 * predefined command patterns using regular expressions, and then calls the
 * appropriate handler function for the matched command. If the input does not
 * match any known command, it checks if it's a regular message and handles it
 * accordingly. It also provides feedback to the user for incorrect command usage
 * or unknown commands.
 *
 * @param connection A pointer to the `ConnectionClient` object for sending messages to the server.
 * @param messagesToConfirm A reference to a vector of `MessageToConfirm` structures,
 * used for tracking messages sent via UDP that require confirmation.
 */
void handleInput(ConnectionClient *connection, std::vector<MessageToConfirm> &messagesToConfirm);

/**
 * @brief Handles the /auth command input from the user.
 *
 * This function extracts the username, secret, and display name from the matched
 * command arguments and sends an authentication message to the server using the
 * `ConnectionClient::SendAuth` method. If the protocol is UDP, it also adds the
 * sent message to the `messagesToConfirm` vector for reliable delivery.
 *
 * @param connection A pointer to the `ConnectionClient` object for sending the authentication message.
 * @param match The `smatch` object containing the captured groups from the regex match.
 * @param messagesToConfirm A reference to a vector of `MessageToConfirm` structures for UDP handling.
 */
void handleAuthCommandInput(ConnectionClient *connection, const std::smatch &match,
                            std::vector<MessageToConfirm> &messagesToConfirm);

/**
 * @brief Handles the /join command input from the user.
 *
 * This function extracts the channel ID from the matched command arguments and
 * sends a join channel message to the server using the `ConnectionClient::SendJoin`
 * method. If the protocol is UDP, it also adds the sent message to the
 * `messagesToConfirm` vector for reliable delivery.
 *
 * @param connection A pointer to the `ConnectionClient` object for sending the join message.
 * @param match The `smatch` object containing the captured channel ID from the regex match.
 * @param messagesToConfirm A reference to a vector of `MessageToConfirm` structures for UDP handling.
 */
void handleJoinCommandInput(ConnectionClient *connection, const std::smatch &match,
                            std::vector<MessageToConfirm> &messagesToConfirm);


/**
 * @brief Handles regular message input from the user (not starting with '/').
 *
 * This function takes the user's message and sends it to the server using the
 * `ConnectionClient::SendMsg` method. If the protocol is UDP, it also adds the
 * sent message to the `messagesToConfirm` vector for reliable delivery.
 *
 * @param connection A pointer to the `ConnectionClient` object for sending the message.
 * @param message The string containing the user's message.
 * @param messagesToConfirm A reference to a vector of `MessageToConfirm` structures for UDP handling.
 */
void handleMessageInput(ConnectionClient *connection, const std::string &line, std::vector<MessageToConfirm> &messagesToConfirm);

/**
 * @brief Handles the /rename command input from the user.
 *
 * This function extracts the new display name from the matched command arguments
 * and calls the `ConnectionClient::Rename` method to request a name change on the server.
 * Note: Rename operations might not require confirmation in the same way as other messages,
 * so it's not added to `messagesToConfirm` here.
 *
 * @param connection A pointer to the `ConnectionClient` object for requesting the rename.
 * @param match The `smatch` object containing the captured new display name.
 */
void handleRenameCommandInput(ConnectionClient *connection, const std::smatch &match);

/**
 * @brief Handles the /exit command input from the user.
 *
 * This function sends a "bye" message to the server using the
 * `ConnectionClient::SendBye` method to initiate a graceful disconnection.
 * If the protocol is UDP, it adds the "bye" message to the `messagesToConfirm`
 * vector for reliable delivery. If the connection is in the initial `Start`
 * state, it directly transitions to the `End` state without sending a "bye".
 *
 * @param connection A pointer to the `ConnectionClient` object for sending the bye message.
 * @param messagesToConfirm A reference to a vector of `MessageToConfirm` structures for UDP handling.
 */
void handleExitCommandInput(ConnectionClient *connection, std::vector<MessageToConfirm> &messagesToConfirm);

/**
 * @brief Handles the /help command input from the user.
 */
void handleHelpCommandInput();



#endif  //USER_INPUT_HPP
