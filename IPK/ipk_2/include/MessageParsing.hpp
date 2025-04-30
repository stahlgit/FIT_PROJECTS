#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <cstdint>
#include <vector>
#include <string>
using namespace std;

//  (Scoped Enumeration)
enum class Protocol {
    TCP,
    UDP
};

enum MessageType : uint8_t
{
    CONFIRM = 0x00,
    REPLY	= 0x01,
    AUTH	= 0x02,
    JOIN	= 0x03,
    MSG		= 0x04,
    ERR		= 0xFE,
    PING    = 0xFD,
    BYE		= 0xFF,
};

/**
 * @brief Base Interface for Messages
 */
class MessageParsing {
public:
    uint16_t messageId = 0;

    virtual ~MessageParsing() = default;

    virtual uint8_t type() const = 0;

    /**
    * @brief Serializes the message into a byte vector.
    *
    * This is a virtual function that can be overridden by derived classes to implement
    * protocol-specific serialization logic. For UDP, it adds the message type and message ID
    * to the beginning of the buffer.
    *
    * @param buffer The byte vector to serialize the message into.
    * @param protocol The communication protocol being used (TCP or UDP).
     */
    virtual void serialize(std::vector<uint8_t> &buffer, const Protocol &protocol) const;

    /**
     * @brief Deserializes the message from a byte vector.
     * @param buffer The byte vector to deserialize the message from.
     * @param protocol The communication protocol being used (TCP or UDP).
    * @return true if the deserialization of the common parts was successful, false otherwise.
     */
    virtual bool deserialize(const std::vector<uint8_t> &buffer, const Protocol &protocol);
protected:
    /**
     * @brief Constructor for the MessageParsing base class.
     *
     * Initializes the message ID for messages, primarily used for UDP message tracking.
     *
     * @param messageId The unique identifier for the message.
     */
    explicit MessageParsing(uint16_t messageId);
};

/**
 * @brief Class representing Confirm Message
 */
class ConfirmMessage : public MessageParsing {
public:
    /**
     * @brief Default constructor for the ConfirmMessage.
     *
     * Initializes a ConfirmMessage with a default message ID of 0.
     */
    ConfirmMessage();

    /**
     * @brief Constructor for the ConfirmMessage with a specific message ID.
     *
     * Initializes a ConfirmMessage with the given message ID.
     *
     * @param messageId The ID of the message being confirmed.
     */
    explicit ConfirmMessage(uint16_t messageId);

    /**
     * @brief Returns the message type identifier for ConfirmMessage.
     *
     * @return The message type identifier (0x00).
     */
    uint8_t type() const override;

    /**
     * @brief Serializes the ConfirmMessage into a byte vector.
     *
     * For TCP, this operation is not supported and throws a runtime error.
     * For UDP, it calls the base class serialization to include the message type and ID.
     *
     * @param buffer The byte vector to serialize the message into.
     * @param protocol The communication protocol being used (TCP or UDP).
     * @throws runtime_error if called with TCP protocol.
     */
    void serialize(std::vector<uint8_t> &buffer, const Protocol &protocol) const override;
    /**
     * @brief Deserializes the ConfirmMessage from a byte vector.
     *
     * For TCP, this operation returns false as ConfirmMessage is not used with TCP.
     * For UDP, it checks the buffer size and message type before calling the base class deserialization.
     *
     * @param buffer The byte vector to deserialize the message from.
     * @param protocol The communication protocol being used (TCP or UDP).
     * @return true if deserialization was successful and the message type is correct, false otherwise.
     */
    bool deserialize(const std::vector<uint8_t> &buffer, const Protocol &protocol) override;
};

/**
 * @brief Class representing Reply Message
 */
class ReplyMessage : public MessageParsing {
public:
    bool result;
    uint16_t refMessageId;
    std::string messageContents;

    /**
     * @brief Default constructor for the ReplyMessage.
     *
     * Initializes a ReplyMessage with default values for its members.
     */
    ReplyMessage();

    /**
     * @brief Returns the message type identifier for ReplyMessage.
     *
     * @return The message type identifier (0x01).
     */
    uint8_t type() const override;

    /**
     * @brief Serializes the ReplyMessage into a byte vector.
     *
     * For TCP, it formats the reply as "REPLY (OK|ERROR) IS messageContents\r\n".
     * For UDP, it includes the base class serialization (type and ID) followed by the
     * message contents (null-terminated).
     *
     * @param buffer The byte vector to serialize the message into.
     * @param protocol The communication protocol being used (TCP or UDP).
     */
    void serialize(std::vector<uint8_t> &buffer, const Protocol &protocol) const override;

    /**
     * @brief Deserializes the ReplyMessage from a byte vector.
     *
     * For TCP, it uses a regular expression to parse the "REPLY ..." format.
     * For UDP, it checks the message type and extracts the result, referenced message ID,
     * and message contents (up to the null terminator).
     *
     * @param buffer The byte vector to deserialize the message from.
     * @param protocol The communication protocol being used (TCP or UDP).
     * @return true if deserialization was successful and the message type is correct, false otherwise.
     */
    bool deserialize(const std::vector<uint8_t> &buffer, const Protocol &protocol) override;
};

/**
 * @brief Class representing Authorization Message
 */
class AuthMessage : public MessageParsing {
public:
    std::string username;
    std::string displayName;
    std::string secret;

    /**
     * @brief Default constructor for the AuthMessage.
     */
    AuthMessage();

    /**
     * @brief Constructor for the AuthMessage with authentication details.
     *
     * @param username The username for authentication.
     * @param displayName The desired display name.
     * @param secret The authentication secret.
     */
    AuthMessage(std::string username, std::string displayName, std::string secret);

    /**
     * @brief Returns the message type identifier for AuthMessage.
     *
     * @return The message type identifier (0x02).
     */
    uint8_t type() const override;


    /**
     * @brief Serializes the AuthMessage into a byte vector.
     *
     * For TCP, it formats the authentication as "AUTH username AS displayName USING secret\r\n".
     * For UDP, it includes the base class serialization followed by the username, display name,
     * and secret, each null-terminated.
     *
     * @param buffer The byte vector to serialize the message into.
     * @param protocol The communication protocol being used (TCP or UDP).
     */
    void serialize(std::vector<uint8_t> &buffer, const Protocol &protocol) const override;

    /**
     * @brief Deserializes the AuthMessage from a byte vector.
     *
     * For TCP, it uses a regular expression to parse the "AUTH ..." format.
     * For UDP, it checks the message type and extracts the username, display name, and secret
     * by reading up to the null terminators.
     *
     * @param buffer The byte vector to deserialize the message from.
     * @param protocol The communication protocol being used (TCP or UDP).
     * @return true if deserialization was successful and the message type is correct, false otherwise.
     */
    bool deserialize(const std::vector<uint8_t> &buffer, const Protocol &protocol) override;
};


/**
 * @brief Class representing Join Message
 */
class JoinMessage : public MessageParsing {
public:
    std::string channelId;
    std::string displayName;

    /**
     * @brief Default constructor for the JoinMessage.
     */
    JoinMessage();
    /**
     * @brief Constructor for the AuthMessage with authentication details.
     *
     * @param channelId Identification of Channel on server
     * @param displayName The desired display name.
     */
    JoinMessage(std::string channelId, std::string displayName);

    /**
     * @brief Returns the message type identifier for JoinMessage.
     *
     * @return The message type identifier (0x03).
     */
    uint8_t type() const override;

    /**
     * @brief Serializes the JoinMessage into a byte vector.
     *
     * For TCP, it formats the join request as "JOIN channelId AS displayName\r\n".
     * For UDP, it includes the base class serialization followed by the channel ID
     * and display name, each null-terminated.
     *
     * @param buffer The byte vector to serialize the message into.
     * @param protocol The communication protocol being used (TCP or UDP).
     */
    void serialize(std::vector<uint8_t> &buffer, const Protocol &protocol) const override;

    /**
     * @brief Deserializes the JoinMessage from a byte vector.
     *
     * This implementation currently returns false for both TCP and UDP as the server
     * typically does not send a JoinMessage back to the client. This might be used
     * for internal processing or a different communication flow.
     *
     * @param buffer The byte vector to deserialize the message from.
     * @param protocol The communication protocol being used (TCP or UDP).
     * @return false, as server typically doesn't send this back.
     */
    bool deserialize(const std::vector<uint8_t> &buffer, const Protocol &protocol) override;
};

/**
 * @brief Class representing Chat Message
 */
class MsgMessage : public MessageParsing {
public:
    std::string displayName;
    std::string messageContents;

    /**
    * @brief Default constructor for the MsgMessage.
     */
    MsgMessage();

    /**
     * @brief Constructor for the MsgMessage with sender's display name and message content.
     *
     * @param displayName The display name of the user sending the message.
     * @param messageContents The content of the chat message.
     */
    MsgMessage(std::string displayName, std::string messageContents);

    /**
     * @brief Returns the message type identifier for MsgMessage.
     *
     * @return The message type identifier (0x04).
     */
    uint8_t type() const override;

    /**
     * @brief Serializes the MsgMessage into a byte vector.
     *
     * For TCP, it formats the message as "MSG FROM displayName IS messageContents\r\n".
     * For UDP, it includes the base class serialization followed by the sender's
     * display name and the message content, each null-terminated.
     *
     * @param buffer The byte vector to serialize the message into.
     * @param protocol The communication protocol being used (TCP or UDP).
     */
    void serialize(std::vector<uint8_t> &buffer, const Protocol &protocol) const override;

    /**
     * @brief Deserializes the MsgMessage from a byte vector.
     *
     * For TCP, it uses a regular expression to parse the "MSG FROM ..." format.
     * For UDP, it checks the message type and extracts the sender's display name
     * and the message content by reading up to the null terminators.
     *
     * @param buffer The byte vector to deserialize the message from.
     * @param protocol The communication protocol being used (TCP or UDP).
     * @return true if deserialization was successful and the message type is correct, false otherwise.
     */
    bool deserialize(const std::vector<uint8_t> &buffer, const Protocol &protocol) override;
};

/**
 * @brief Class representing Error Message
 */
class ErrMessage : public MessageParsing {
public:
    std::string displayName;
    std::string messageContents;

    /**
     * @brief Default constructor for the ErrMessage.
     */
    ErrMessage();
    /**
     * @brief Returns the message type identifier for ErrMessage.
     *
     * @return The message type identifier (0xFE).
     */
    uint8_t type() const override;

    /**
     * @brief Serializes the ErrMessage into a byte vector.
     *
     * For TCP, it formats the error message as "ERR FROM displayName IS messageContents\r\n".
     * For UDP, it includes the base class serialization followed by the source's
     * display name and the error message content, each null-terminated.
     *
     * @param buffer The byte vector to serialize the message into.
     * @param protocol The communication protocol being used (TCP or UDP).
     */
    void serialize(std::vector<uint8_t> &buffer, const Protocol &protocol) const override;


    /**
     * @brief Deserializes the ErrMessage from a byte vector.
     *
     * For TCP, it uses a regular expression to parse the "ERR FROM ..." format.
     * For UDP, it checks the message type and extracts the source's display name
     * and the error message content by reading up to the null terminators.
     *
     * @param buffer The byte vector to deserialize the message from.
     * @param protocol The communication protocol being used (TCP or UDP).
     * @return true if deserialization was successful and the message type is correct, false otherwise.
     */
    bool deserialize(const std::vector<uint8_t> &buffer, const Protocol &protocol) override;
};

/**
 * @brief Class representing Bye Message
 */
class ByeMessage : public MessageParsing {
public:
    /**
     * @brief Default constructor for the ByeMessage.
     */
    ByeMessage();

    /**
     * @brief Returns the message type identifier for ErrMessage.
     *
     * @return The message type identifier (0xFF).
     */
    uint8_t type() const override;

    /**
     * @brief Serializes the ByeMessage into a byte vector.
     *
     * For TCP, it formats the disconnection message as "BYE\r\n".
     * For UDP, it only includes the base class serialization (type and ID) as no
     * additional data is needed for a BYE message in this protocol.
     *
     * @param buffer The byte vector to serialize the message into.
     * @param protocol The communication protocol being used (TCP or UDP).
     */
    void serialize(std::vector<uint8_t> &buffer, const Protocol &protocol) const override;


    /**
     * @brief Deserializes the ByeMessage from a byte vector.
     *
     * For TCP, it checks if the received buffer exactly matches "BYE\r\n" (5 bytes).
     * For UDP, it checks if the buffer size is exactly 3 bytes (type and message ID).
     *
     * @param buffer The byte vector to deserialize the message from.
     * @param protocol The communication protocol being used (TCP or UDP).
     * @return true if deserialization was successful and the buffer content/size is correct, false otherwise.
     */
    bool deserialize(const std::vector<uint8_t> &buffer, const Protocol &protocol) override;
};

/**
 * @brief Class representing Bye Message
 */
class PingMessage : public MessageParsing {
public:
    /**
     * @brief Default constructor for the ConfirmMessage.
     *
     * Initializes a ConfirmMessage with a default message ID of 0.
     */
    PingMessage();

    /**
     * @brief Constructor for the PingMessage with a specific message ID.
     *
     * Initializes a PingMessage with the given message ID.
     *
     * @param messageId The ID of the message being confirmed.
     */
    explicit PingMessage(uint16_t messageId);

    /**
     * @brief Returns the message type identifier for ConfirmMessage.
     *
     * @return The message type identifier (0x00).
     */
    uint8_t type() const override;

    /**
     * @brief Serializes the PingMessage into a byte vector.
     *
     * For TCP, this operation is not supported and throws a runtime error.
     * For UDP, it calls the base class serialization to include the message type and ID.
     *
     * @param buffer The byte vector to serialize the message into.
     * @param protocol The communication protocol being used (TCP or UDP).
     * @throws runtime_error if called with TCP protocol.
     */
    void serialize(std::vector<uint8_t> &buffer, const Protocol &protocol) const override;
    /**
     * @brief Deserializes the PingMessage from a byte vector.
     *
     * For TCP, this operation returns false as PingMessage is not used with TCP.
     * For UDP, it checks the buffer size and message type before calling the base class deserialization.
     *
     * @param buffer The byte vector to deserialize the message from.
     * @param protocol The communication protocol being used (TCP or UDP).
     * @return true if deserialization was successful and the message type is correct, false otherwise.
     */
    bool deserialize(const std::vector<uint8_t> &buffer, const Protocol &protocol) override;
};




struct MessageToConfirm {
    MessageParsing *message;
    time_t scheduledRetransmitTimestamp;
    uint8_t retransmitCount;
};

/**
 * --- Help Functions ---
**/

string computeString(const vector<uint8_t> &buffer, size_t index);
/**
 * @brief Helper function to append a string to a byte vector.
 *
 * Each character of the string is added as a single byte to the vector.
 *
 * @param buffer The byte vector to append to.
 * @param str The string to append.
 */
void appendString(vector<uint8_t> &buffer, const string &str);

/**
 * @brief Checks if there are any messages in the confirmation list that have failed
 * all retransmission attempts.
 *
 * @param messagesToConfirm The vector of messages awaiting confirmation.
 * @return true if there are failed messages (retransmitCount is 0), false otherwise.
 */
bool areThereFailedMessages(const std::vector<MessageToConfirm> &messagesToConfirm);

/**
 * @brief Sorts the vector of messages to confirm based on their scheduled retransmit timestamp.
 *
 * This allows the client to prioritize retransmitting messages that are due sooner.
 *
 * @param messagesToConfirm The vector of messages to be sorted.
 */
void sortMessagesToConfirm(std::vector<MessageToConfirm> &messagesToConfirm);

/**
 * @brief Removes a confirmed message from the vector of messages awaiting confirmation.
 *
 * This function is called when an acknowledgment (e.g., a ConfirmMessage) is received
 * for a previously sent message.
 *
 * @param messagesToConfirm The vector of messages awaiting confirmation.
 * @param messageId The ID of the message that has been confirmed.
 */
void confirmMessage(std::vector<MessageToConfirm> &messagesToConfirm, uint16_t messageId);

#endif //MESSAGE_HPP
