#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <vector>
#include <netinet/in.h>
#include <string>
#include "MessageParsing.hpp"

/**
 * @brief Represent states in FSM
 */
enum class ConnectionState {
    Start,
    Auth,
    Open,
    Error,
    End
};

/**
 * @brief Abstract base class for client-server communication.
 *
 * This class defines the common interface and functionality for establishing and
 * managing a connection with a server, regardless of the underlying transport
 * protocol (TCP or UDP). It provides methods for sending and receiving messages,
 * handling different connection states, and managing connection parameters.
 */
class ConnectionClient {
public:
    ConnectionState state = ConnectionState::Start; // starting state
    /**
     * @brief Constructor for TCP-based connections.
     * @param address The server's address information.
     */
    explicit ConnectionClient(const struct sockaddr_in &address);

    /**
      * @brief Constructor for UDP-based connections, allowing specification of retransmission parameters.
      * @param address The server's address information.
      * @param retransmitTimeout The timeout in milliseconds before retransmitting a message.
      * @param retransmitCount The maximum number of times to retransmit a message.
      */
    ConnectionClient(struct sockaddr_in &address, uint16_t retransmitTimeout, uint8_t retransmitCount);

    /**
     * @brief Destructor for Connection Client
     */
    ~ConnectionClient();

    /**
     * @brief Returns the protocol currently used by the connection.
     * @return The `Protocol` enum value indicating TCP or UDP.
     */
    Protocol GetProtocol();
    /**
     * @brief Handles incoming data from the socket and attempts to parse it into a Message object.
     * @param message A pointer to a `MessageParsing` pointer where the parsed message will be stored.
     * @return `true` if data was successfully read and a valid message was likely received, `false` otherwise.
     */
    bool HandleIncomingData(MessageParsing **message);

    /**
     * @brief Sends an authentication message to the server.
     * @param username The username for authentication.
     * @param displayName The desired display name.
     * @param secret The authentication secret.
     * @return A pointer to the created and sent `AuthMessage` object, or `nullptr` on failure.
     */
    AuthMessage *SendAuth(const std::string &username, const std::string &displayName, const std::string &secret);

    /**
     * @brief Sends a confirmation message for a received message (primarily used with UDP).
     * @param messageId The ID of the message being acknowledged.
     */
    void SendConfirm(uint16_t messageId);

    /**
     * @brief Sends a message to the server.
     * @param message The `MessageParsing` object to send (the content of the message)
     */
    void SendMessage(MessageParsing &message);


    /**
     * @brief Sends a join channel message to the server.
     * @param channelId The ID of the channel to join.
     * @return A pointer to the created and sent `JoinMessage` object, or `nullptr` on failure.
     */
    JoinMessage *SendJoin(const std::string &channelId);

    /**
     * @brief Sends a regular chat message to the server.
     * @param content The content of the message.
     * @return A pointer to the created and sent `MsgMessage` object, or `nullptr` on failure.
     */
    MsgMessage *SendMsg(const std::string &content);

    /**
     * @brief Sends a "bye" message to the server to indicate disconnection.
     * @return A pointer to the created and sent `ByeMessage` object, or `nullptr` on failure.
     */
    ByeMessage *SendBye();

    /**
     * @brief Requests a change of the client's display name on the server.
     * @param newName The new display name to use.
     */
    void Rename(const std::string &newName);


    /**
     * @brief Returns the underlying socket file descriptor.
     * @return The integer representing the socket file descriptor.
     */
    int GetSocket() const;


    /**
     * @brief Returns the retransmit timeout value (used for UDP).
     * @return The retransmit timeout in milliseconds.
     */
    uint16_t GetRetransmitTimeout() const;

    /**
     * @brief Returns the maximum number of retransmissions allowed (used for UDP).
     * @return The maximum retransmit count.
     */
    uint8_t GetRetransmitCount() const;



private:
    // The socket file descriptor.
    int clientSocket = -1; // uninitialized

    // The address of the server.
    struct sockaddr_in dstAddress; // In UDP port changes after connection

    // The Display Name of user in Chat
    std::string currentDisplayName;

    // The protocol used by the connection.
    Protocol protocol;

    /** UDP specific **/
    // The retransmit timeout in milliseconds.
    uint16_t resendTimeout;

    // The number of retransmissions.
    uint8_t retransmitCount_;

    // The message ID counter, used to uniquely identify sent messages.
    uint16_t currentMessageId = 0;

    /** @brief Attempts to deserialize the byte buffer into different message types (Confirm, Reply, Msg, Err)
     * by calling the corresponding handler functions.
     * @param buffer The byte vector containing the received message.
     * @param message A pointer to a `MessageParsing` pointer to store the deserialized message.
     * @return `true` if deserialization into a known message type was successful, `false` otherwise.
     */
    bool HandleIncomingMessage(const std::vector<uint8_t> &buffer, MessageParsing **message);

    /**
     * @brief Handles an incoming confirmation message.
     * @param buffer The byte vector containing the received message.
     * @param message A pointer to a `MessageParsing` pointer to store the deserialized message.
     * @return `true` if the buffer represents a confirmation message and was handled, `false` otherwise.
     */
    bool HandleIncomingConfirmMessage(const std::vector<uint8_t> &buffer, MessageParsing **message);

    /**
     * @brief Handles an incoming reply message.
     * @param buffer The byte vector containing the received message.
     * @param message A pointer to a `MessageParsing` pointer to store the deserialized message.
     * @return `true` if the buffer represents a reply message and was handled, `false` otherwise.
     */
    bool HandleIncomingReplyMessage(const std::vector<uint8_t> &buffer, MessageParsing **message);

    /**
     * @brief Handles an incoming regular chat message.
     * @param buffer The byte vector containing the received message.
     * @param message A pointer to a `MessageParsing` pointer to store the deserialized message.
     * @return `true` if the buffer represents a chat message and was handled, `false` otherwise.
     */
    bool HandleIncomingMsgMessage(const std::vector<uint8_t> &buffer, MessageParsing **message);

    /**
     * @brief Handles an incoming error message from the server.
     * @param buffer The byte vector containing the received message.
     * @param message A pointer to a `MessageParsing` pointer to store the deserialized message.
     * @return `true` if the buffer represents an error message and was handled, `false` otherwise.
     */
    bool HandleIncomingErrMessage(const std::vector<uint8_t> &buffer, MessageParsing **message);


    /**
     * @brief Handles an incoming ping message.
     * @param buffer The byte vector containing the received message.
     * @param message A pointer to a `MessageParsing` pointer to store the deserialized message.
     * @return `true` if the buffer represents a ping message and was handled, `false` otherwise.
     */
    bool HandleIncomingPingMessage(const std::vector<uint8_t> &buffer, MessageParsing **message);

    /**
     * @brief Transmits a byte buffer over the network.
     * Uses `send` for TCP and `sendto` for UDP to transmit the data to the server.
     * @param buffer The byte vector to send.
     */
    void Transmit(const std::vector<uint8_t> &buffer);

};

/**
 * @brief Creates a sockaddr_in structure from an address string and port.
 * Supports both IP addresses and hostnames (via DNS resolution).
 * @param sockaddr The sockaddr_in structure to populate.
 * @param address The IP address or hostname.
 * @param port The port number.
 */
void create_sockaddr_in(sockaddr_in &sockaddr, const std::string &address, uint16_t port);

/**
 * @brief Creates a ConnectionClient object for TCP communication.
 * @param address The server's address (IP or hostname).
 * @param port The server's port number.
 * @return A pointer to the newly created ConnectionClient object.
 */
ConnectionClient *createTCPConnection(const std::string &address, uint16_t port);

/**
 * @brief Creates a ConnectionClient object for UDP communication.
 * @param address The server's address (IP or hostname).
 * @param port The server's port number.
 * @param retransmitTimeout The timeout in milliseconds for UDP retransmissions.
 * @param retransmitCount The maximum number of retransmissions for UDP messages.
 * @return A pointer to the newly created ConnectionClient object.
 */
ConnectionClient *createUDPConnection(const std::string &address, uint16_t port, uint16_t retransmitTimeout,
                                uint8_t retransmitCount);

#endif //CONNECTION_HPP
