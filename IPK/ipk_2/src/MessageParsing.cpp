#include <stdexcept>
#include <regex>
#include <iostream>
// #include "Message.hpp"
#include "../include/MessageParsing.hpp"

using namespace std;

// REGEX Expression (better to have them here, for Encapsulation, Reduced Coupling, Header Pollution, ... )
regex tcpReplyRegex(R"(REPLY (OK|NOK) IS ([\x21-\x7E ]{1,1400})\r\n)");
regex tcpAuthRegex(R"(AUTH [a-zA-Z0-9-]{1,20} AS [\x21-\x7E]{1,20} USING [a-zA-Z0-9-]{1,128})");
regex tcpMsgRegex(R"(MSG FROM ([\x21-\x7E]{1,20}) IS ([\x21-\x7E ]{1,1400})\r\n)");
regex tcpErrRegex(R"(ERR FROM ([\x21-\x7E]{1,20}) IS ([\x21-\x7E ]{1,1400})\r\n)");

MessageParsing::MessageParsing(uint16_t messageId): messageId(messageId) {}

void MessageParsing::serialize(vector<uint8_t> &buffer, const Protocol &protocol) const {
    if (protocol == Protocol::TCP) {
        // TCP has no special header that's common for each message
    }
    if (protocol == Protocol::UDP) {
        buffer.push_back(this->type());
        buffer.push_back(messageId >> 8); // HIGH byte
        buffer.push_back(messageId & 0xFF); // LOW byte
    }
}

bool MessageParsing::deserialize(const vector<uint8_t> &buffer, const Protocol &protocol) {
    if (protocol == Protocol::TCP) {
        // TCP messages are deserialized directly by their specific classes.
        return true;
    }
    if (protocol == Protocol::UDP) {
        if (buffer.size() < 3){
            return false;
        }
        // Reconstruct messageId
        messageId = buffer[1] << 8 | buffer[2];
        return true;
    }
    return false; // unreachable
}

/**
 * --- CONFIRM Implementation ---
**/

ConfirmMessage::ConfirmMessage() : MessageParsing(0) {}

ConfirmMessage::ConfirmMessage(uint16_t messageId): MessageParsing(messageId) {}

uint8_t ConfirmMessage::type() const {
    return CONFIRM;
}

void ConfirmMessage::serialize(vector<uint8_t> &buffer, const Protocol &protocol) const {
    if (protocol == Protocol::TCP) {
        throw runtime_error("ERROR: TCP doesn't have a ConfirmMessage");
    }
    if (protocol == Protocol::UDP) {
        MessageParsing::serialize(buffer, protocol);
    }
}

bool ConfirmMessage::deserialize(const vector<uint8_t> &buffer, const Protocol &protocol) {
    if (protocol == Protocol::TCP) {
        return false;
    }
    if (protocol == Protocol::UDP) {
        if (buffer.size() != 3){
            return false;
        }
        if (buffer[0] != type()){
            return false;
        }
        if (buffer[0] != type()){
            return false;
        }
        return MessageParsing::deserialize(buffer, protocol);;
    }
    return false; // unreachable
}

/**
 * --- REPLY Implementation ---
**/

ReplyMessage::ReplyMessage() : MessageParsing(0), result(false), refMessageId(0) {}

uint8_t ReplyMessage::type() const {
    return REPLY;
}

void ReplyMessage::serialize(vector<uint8_t> &buffer, const Protocol &protocol) const {
    if (protocol == Protocol::TCP) {
        MessageParsing::serialize(buffer, protocol); // call the base serializations
        appendString(buffer, "REPLY ");
        if (result) appendString(buffer, "OK ");
        else appendString(buffer, "ERROR ");
        appendString(buffer, "IS ");
        appendString(buffer, messageContents);
        appendString(buffer, "\r\n");
    }
    if (protocol == Protocol::UDP) {
        MessageParsing::serialize(buffer, protocol); // add type + ID
        buffer.push_back(result ? 1 : 0);
        buffer.push_back(refMessageId >> 8);
        buffer.push_back(refMessageId & 0xFF);
        appendString(buffer, messageContents);
        buffer.push_back('\0');
    }
}

bool ReplyMessage::deserialize(const vector<uint8_t> &buffer, const Protocol &protocol) {
    if (!MessageParsing::deserialize(buffer, protocol)){
        return false;
    }
    if (protocol == Protocol::TCP) {
        string str(buffer.begin(), buffer.end());
        smatch match;
        if (regex_match(str, match, tcpReplyRegex)) {
            // Set the result based on the matched group.
            if (match[1] == "OK"){
                result = true; // Reply Message inner status
                messageContents = match[2];  // Extract the message content.
                return true;
            }
        }
        return false;
    }
    if (protocol == Protocol::UDP) {
        // check message type
        if (buffer[0] != type()){
            return false;
        }
        // UDP ReplyMessage needs at least type, ID, result, refID, and one char + null.
        if (buffer.size() < 8){
            return false;
        }
        // Extract Boolean result
        result = buffer[3];
        // Extract the referenced message ID.
        refMessageId = buffer[4] << 8 | buffer[5];
        int messageContentsStart = 6;
        int messageContentsEnd = messageContentsStart;
        while (buffer[messageContentsEnd] != '\0') {
            // Find the null terminator for the message contents.
            messageContentsEnd++;
        }
        messageContents = string(buffer.begin() + messageContentsStart, buffer.begin() + messageContentsEnd);
        return true;
    }
    return false;
}

/**
 * --- REPLY Implementation ---
**/

AuthMessage::AuthMessage() : MessageParsing(0) {}

AuthMessage::AuthMessage(string username, string displayName, string secret)
        : MessageParsing(0), username(move(username)), displayName(move(displayName)), secret(move(secret)) {}

uint8_t AuthMessage::type() const {
    return AUTH;
}

void AuthMessage::serialize(vector<uint8_t> &buffer, const Protocol &protocol) const {
    MessageParsing::serialize(buffer, protocol);
    if (protocol == Protocol::TCP) {
        appendString(buffer, "AUTH ");
        appendString(buffer, username);
        appendString(buffer, " AS ");
        appendString(buffer, displayName);
        appendString(buffer, " USING ");
        appendString(buffer, secret);
        appendString(buffer, "\r\n");
    }
    if (protocol == Protocol::UDP) {
        appendString(buffer, username);
        buffer.push_back('\0');
        appendString(buffer, displayName);
        buffer.push_back('\0');
        appendString(buffer, secret);
        buffer.push_back('\0');
    }
}

bool AuthMessage::deserialize(const vector<uint8_t> &buffer, const Protocol &protocol) {
    if (!MessageParsing::deserialize(buffer, protocol)){
        return false;
    }
    if (protocol == Protocol::TCP) {
        string str(buffer.begin(), buffer.end());
        smatch match;
        if (regex_match(str, match, tcpAuthRegex)) {
            username = match[1];
            displayName = match[2];
            secret = match[3];
            return true;
        }
        return false; // TCP string did not match the expected format
    }
    if (protocol == Protocol::UDP) {
        if (buffer[0] != type()){
            return false;
        }
        if (buffer.size() < 7)
        {
            return false;
        }
        size_t usernameStart = 3; // Type + ID
        string result;
        result = computeString(buffer, usernameStart);
        if (result.empty()){
            return false;
        }
        username = result;

        size_t displayNameStart = usernameStart + result.size() + 1;
        result = computeString(buffer, displayNameStart);
        if (result.empty()){
            return false;
        }
        displayName = result;


        size_t secretStart = displayNameStart + result.size() + 1;
        result = computeString(buffer, secretStart);
        if (result.empty()){
            return false;
        }
        secret = result;

        return true;
    }
    return false;
}

/**
 * --- JOIN Implementation ---
**/

JoinMessage::JoinMessage() : MessageParsing(0) {}

JoinMessage::JoinMessage(string channelId, string displayName)
        : MessageParsing(0), channelId(move(channelId)), displayName(move(displayName)) {}

uint8_t JoinMessage::type() const {
    return JOIN;
}

void JoinMessage::serialize(vector<uint8_t> &buffer, const Protocol &protocol) const {
    MessageParsing::serialize(buffer, protocol);
    if (protocol == Protocol::TCP) {
        appendString(buffer, "JOIN ");
        appendString(buffer, channelId);
        appendString(buffer, " AS ");
        appendString(buffer, displayName);
        appendString(buffer, "\r\n");
    }
    if (protocol == Protocol::UDP) {
        appendString(buffer, channelId);
        buffer.push_back('\0');
        appendString(buffer, displayName);
        buffer.push_back('\0');
    }
}

bool JoinMessage::deserialize(const vector<uint8_t> &buffer, const Protocol &protocol) {
    (void) buffer;
    (void) protocol;
    return false;
}

/**
 * --- MSG Implementation ---
**/

MsgMessage::MsgMessage() : MessageParsing(0) {}

MsgMessage::MsgMessage(string displayName, string messageContents)
        : MessageParsing(0), displayName(move(displayName)), messageContents(move(messageContents)) {}

uint8_t MsgMessage::type() const {
    return MSG;
}

void MsgMessage::serialize(vector<uint8_t> &buffer, const Protocol &protocol) const {
    MessageParsing::serialize(buffer, protocol);
    if (protocol == Protocol::TCP) {
        appendString(buffer, "MSG FROM ");
        appendString(buffer, displayName);
        appendString(buffer, " IS ");
        appendString(buffer, messageContents);
        appendString(buffer, "\r\n");
    }
    if (protocol == Protocol::UDP) {
        appendString(buffer, displayName);
        buffer.push_back('\0');
        appendString(buffer, messageContents);
        buffer.push_back('\0');
    }
}

bool MsgMessage::deserialize(const vector<uint8_t> &buffer, const Protocol &protocol) {
    if (!MessageParsing::deserialize(buffer, protocol)){
        return false;
    }
    if (protocol == Protocol::TCP) {
        string str(buffer.begin(), buffer.end());
        smatch match;
        if (regex_match(str, match, tcpMsgRegex)) {
            displayName = match[1];
            messageContents = match[2];
            return true;
        }
        return false;
    }
    if (protocol == Protocol::UDP) {
        if (buffer[0] != type()){
            return false;
        }
        if (buffer.size() < 7){
            return false;
        }
        int displayNameStart = 3;
        int displayNameEnd = displayNameStart;
        while (buffer[displayNameEnd] != '\0') {
            displayNameEnd++; // Find the null terminator for message content.
        }
        displayName = string(buffer.begin() + displayNameStart, buffer.begin() + displayNameEnd);
        int messageContentsStart = displayNameEnd + 1;
        int messageContentsEnd = messageContentsStart;
        while (buffer[messageContentsEnd] != '\0') {
            messageContentsEnd++;  // Find the null terminator for message content.
        }
        messageContents = string(buffer.begin() + messageContentsStart, buffer.begin() + messageContentsEnd);
        return true;
    }
    return false;
}

/**
 * --- ERR Implementation ---
**/
ErrMessage::ErrMessage() : MessageParsing(0) {}

uint8_t ErrMessage::type() const {
    return ERR;
}

void ErrMessage::serialize(vector<uint8_t> &buffer, const Protocol &protocol) const {
    MessageParsing::serialize(buffer, protocol);
    if (protocol == Protocol::TCP) {
        appendString(buffer, "ERR FROM ");
        appendString(buffer, displayName);
        appendString(buffer, " IS ");
        appendString(buffer, messageContents);
        appendString(buffer, "\r\n");
    }
    if (protocol == Protocol::UDP) {
        appendString(buffer, displayName);
        buffer.push_back('\0');
        appendString(buffer, messageContents);
        buffer.push_back('\0');
    }
}

bool ErrMessage::deserialize(const vector<uint8_t> &buffer, const Protocol &protocol) {
    if (!MessageParsing::deserialize(buffer, protocol)){
        return false;
    }
    if (protocol == Protocol::TCP) {
        string str(buffer.begin(), buffer.end());
        smatch match;
        if (regex_match(str, match, tcpErrRegex)) {
            displayName = match[1];
            messageContents = match[2];
            return true;
        }
        return false;
    }
    if (protocol == Protocol::UDP) {
        if (buffer[0] != type()){
            return false;
        }
        if (buffer.size() < 7){
            return false;
        }
        int displayNameStart = 3;
        int displayNameEnd = displayNameStart;
        while (buffer[displayNameEnd] != '\0') {
            displayNameEnd++;
        }
        displayName = string(buffer.begin() + displayNameStart, buffer.begin() + displayNameEnd);
        int messageContentsStart = displayNameEnd + 1;
        int messageContentsEnd = messageContentsStart;
        while (buffer[messageContentsEnd] != '\0') {
            messageContentsEnd++;
        }
        messageContents = string(buffer.begin() + messageContentsStart, buffer.begin() + messageContentsEnd);
        return true;
    }
    return false;
}

/**
 * --- BYE Implementation ---
**/

ByeMessage::ByeMessage() : MessageParsing(0) {}

uint8_t ByeMessage::type() const {
    return BYE;
}

void ByeMessage::serialize(vector<uint8_t> &buffer, const Protocol &protocol) const {
    MessageParsing::serialize(buffer, protocol);
    if (protocol == Protocol::TCP) {
        appendString(buffer, "BYE\r\n");
    }
    if (protocol == Protocol::UDP) {
        // no additional data
    }
}

bool ByeMessage::deserialize(const vector<uint8_t> &buffer, const Protocol &protocol) {
    if (!MessageParsing::deserialize(buffer, protocol)){
        return false;
    }
    if (protocol == Protocol::TCP) {
        return buffer.size() == 5;
    }
    if (protocol == Protocol::UDP) {
        return buffer.size() == 3;
    }
    return false;
}

/**
 * --- PING Implementation
 */
PingMessage::PingMessage() : MessageParsing(0){}

PingMessage::PingMessage(uint16_t messageId): MessageParsing(messageId) {}

uint8_t PingMessage::type() const
{
    return PING;
}

void PingMessage::serialize(std::vector<uint8_t>& buffer, const Protocol& protocol) const
{
    if (protocol == Protocol::TCP) {
        throw runtime_error("ERROR: TCP doesn't have a PingMessage");
    }
    if (protocol == Protocol::UDP) {
        MessageParsing::serialize(buffer, protocol);
    }
}

bool PingMessage::deserialize(const vector<uint8_t> &buffer, const Protocol &protocol) {
    if (protocol == Protocol::TCP) {
        return false;
    }
    if (protocol == Protocol::UDP) {
        if (buffer.size() != 3){
            return false;
        }
        if (buffer[0] != type()){
            return false;
        }
        if (buffer[0] != type()){
            return false;
        }
        return MessageParsing::deserialize(buffer, protocol);;
    }
    return false; // unreachable
}
/**
 * --- Help Functions ---
**/
string computeString(const vector<uint8_t> &buffer, size_t index)
{
    size_t start = index;
    while (index < buffer.size() && buffer[index] != '\0'){
        index++;
    }
    if (index == buffer.size()){
        return "";
    }
    return string(buffer.begin() + start, buffer.begin() + index);
}


void appendString(vector<uint8_t> &buffer, const string &str) {
    for (char c: str) {
        buffer.push_back(c);
    }
}

bool areThereFailedMessages(const vector<MessageToConfirm> &messagesToConfirm) {
    return any_of(messagesToConfirm.begin(), messagesToConfirm.end(),[](const MessageToConfirm &messageToConfirm)
        {
            return messageToConfirm.retransmitCount == 0;
        });
}

void sortMessagesToConfirm(vector<MessageToConfirm> &messagesToConfirm) {
    static auto comparator = [](const MessageToConfirm &a, const MessageToConfirm &b) {
        return a.scheduledRetransmitTimestamp < b.scheduledRetransmitTimestamp;
    };
    sort(messagesToConfirm.begin(), messagesToConfirm.end(), comparator);
}

void confirmMessage(vector<MessageToConfirm> &messagesToConfirm, uint16_t messageId) {
    auto iterator = find_if(messagesToConfirm.begin(), messagesToConfirm.end(),[messageId](const MessageToConfirm &messageToConfirm)
         {
             return messageToConfirm.message->messageId == messageId;
         });
    if (iterator != messagesToConfirm.end()) {
        messagesToConfirm.erase(iterator);
    }
}