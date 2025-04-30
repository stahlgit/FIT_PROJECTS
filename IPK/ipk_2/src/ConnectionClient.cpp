#include <stdexcept>
#include <netdb.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>

#include "../include/ConnectionClient.hpp"

using namespace std;

ConnectionClient::ConnectionClient(const sockaddr_in &address)
        : dstAddress(address), protocol(Protocol::TCP), resendTimeout(0), retransmitCount_(0)
{
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == -1) {
        throw runtime_error("ERROR: Failed to create a socket");
    }
    int result = connect(clientSocket, (struct sockaddr *) &address, sizeof(address));
    if (result == -1) {
        throw runtime_error("ERROR: Failed to connect to server");
    }
}

ConnectionClient::ConnectionClient(sockaddr_in &address, uint16_t retransmitTimeout, uint8_t retransmitCount)
        : dstAddress(address), protocol(Protocol::UDP), resendTimeout(retransmitTimeout), retransmitCount_(retransmitCount)
{
    clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSocket == -1) {
        throw runtime_error("ERROR: Failed to create a socket");
    }
}

ConnectionClient::~ConnectionClient()
{
    if (clientSocket != -1) {
        close(clientSocket);
    }
}

Protocol ConnectionClient::GetProtocol()
{
    return protocol;
}

bool ConnectionClient::HandleIncomingData(MessageParsing **message)
{
    char receiveBuffer[2048];
    ssize_t bytesRead = -1;
    if (protocol == Protocol::TCP) {
        // data over TCP (reliable connection)
        bytesRead = recv(clientSocket, receiveBuffer, sizeof(receiveBuffer), 0);
    } else if (protocol == Protocol::UDP) {
        // Receive data over UDP, also get the sender's address.
        sockaddr_in fromAddr = {};
        socklen_t fromLen = sizeof(fromAddr);
        bytesRead = recvfrom(clientSocket, receiveBuffer, sizeof(receiveBuffer), 0, (struct sockaddr *) &fromAddr,
                             &fromLen);
        // Update the destination address to the sender's address for future replies.
        dstAddress.sin_port = fromAddr.sin_port;
    }
    if (bytesRead == -1) {
        throw runtime_error("Error reading from socket");
    } else if (bytesRead > 0) {
        // Create a byte vector from the received data.
        // detect if it is a valid message
        vector<uint8_t> buffer(receiveBuffer, receiveBuffer + bytesRead);
        return HandleIncomingMessage(buffer, message);
    } else {
        return false; // no data received
    }
}

AuthMessage *ConnectionClient::SendAuth(const string &username, const string &displayName, const string &secret)
{
    // Check if authentication is allowed in the current state.
    if (state != ConnectionState::Start && state != ConnectionState::Auth) {
        cerr << "ERR: cannot currently authenticate" << endl;
        return nullptr;
    }
    // Update FSM state to Authentication
    state = ConnectionState::Auth;

    currentDisplayName = displayName;

    // creating auth message
    auto *message = new AuthMessage(username, currentDisplayName, secret);
    message->messageId = currentMessageId++;
    vector<uint8_t> buffer;
    message->serialize(buffer, protocol); // serialize the message to correct format for server

    Transmit(buffer);
    return message;
}

void ConnectionClient::SendConfirm(uint16_t messageId)
{
    auto confirmMessage = new ConfirmMessage(messageId);
    vector<uint8_t> buffer;
    confirmMessage->serialize(buffer, protocol);
    Transmit(buffer);
}

void ConnectionClient::SendMessage(MessageParsing &message)
{
    vector<uint8_t> buffer;
    message.serialize(buffer, protocol);
    message.messageId = currentMessageId++;
    Transmit(buffer);
}

JoinMessage *ConnectionClient::SendJoin(const string &channelId)
{
    if (state != ConnectionState::Open) {
        cerr << "ERROR: cannot currently join channel" << endl;
        return nullptr;
    }

    auto message = new JoinMessage(channelId, currentDisplayName);
    message->messageId = currentMessageId++;
    vector<uint8_t> buffer;
    message->serialize(buffer, protocol);

    Transmit(buffer);
    return message;
}

MsgMessage *ConnectionClient::SendMsg(const string &content)
{
    if (state != ConnectionState::Open) {
        cerr << "ERROR: cannot currently send message" << endl;
        return nullptr;
    }

    auto message = new MsgMessage(currentDisplayName, content);
    message->messageId = currentMessageId++;
    vector<uint8_t> buffer;
    message->serialize(buffer, protocol);

    Transmit(buffer);
    return message;
}

ByeMessage *ConnectionClient::SendBye()
{
    // If User is not connected to user, it's unnecessary to send Bye
    if (state == ConnectionState::Start) {
        cerr << "ERR: cannot currently send bye because not yet connected" << endl;
        return nullptr;
    }
    // set state to signal end of program
    state = ConnectionState::End;

    auto message = new ByeMessage();
    message->messageId = currentMessageId++;
    vector<uint8_t> buffer;
    message->serialize(buffer, protocol);

    Transmit(buffer);
    return message;
}

void ConnectionClient::Rename(const string &newName)
{
    currentDisplayName = newName;
}

int ConnectionClient::GetSocket() const
{
    return clientSocket;
}

uint16_t ConnectionClient::GetRetransmitTimeout() const
{
    return resendTimeout;
}

uint8_t ConnectionClient::GetRetransmitCount() const
{
    return retransmitCount_;
}



bool ConnectionClient::HandleIncomingMessage(const vector<uint8_t> &buffer, MessageParsing **message)
{
    return HandleIncomingConfirmMessage(buffer, message) ||
        HandleIncomingReplyMessage(buffer, message) ||
        HandleIncomingMsgMessage(buffer, message) ||
        HandleIncomingPingMessage(buffer, message) ||
        HandleIncomingErrMessage(buffer, message);

}

bool ConnectionClient::HandleIncomingConfirmMessage(const vector<uint8_t> &buffer, MessageParsing **message)
{
    auto confirmMessage = new ConfirmMessage(); // new message
    bool success = confirmMessage->deserialize(buffer, protocol);
    if (success) {
        *message = confirmMessage;
    }else
    {
        delete confirmMessage; // memory safety
    }
    return success;
}

bool ConnectionClient::HandleIncomingReplyMessage(const vector<uint8_t> &buffer, MessageParsing **message)
{
    auto replyMessage = new ReplyMessage();
    bool success = replyMessage->deserialize(buffer, protocol);
    if (success) {
        // Update state based on the reply during authentication.
        if (state == ConnectionState::Auth && replyMessage->result) {
            state = ConnectionState::Open;
        } else if (state == ConnectionState::Auth && !replyMessage->result) {
            // Stay in Auth state on failed authentication.
        } else if (state == ConnectionState::End || state == ConnectionState::Open) {
            // Ignore replies in End or Open states (might be delayed).
        } else {
            state = ConnectionState::Error;
        }
        *message = replyMessage;
    }
    return success;
}

bool ConnectionClient::HandleIncomingMsgMessage(const vector<uint8_t> &buffer, MessageParsing **message)
{
    auto msgMessage = new MsgMessage();
    bool success = msgMessage->deserialize(buffer, protocol);
    if (success) {
        // Check if receiving messages is allowed in the current state.
        if (state != ConnectionState::Open) {
            cerr << "ERR: cannot currently receive message" << endl;
            state = ConnectionState::Error;
            return true; // Indicate successful deserialization but an error condition.
        }
        *message = msgMessage;
    }
    return success;
}

bool ConnectionClient::HandleIncomingErrMessage(const vector<uint8_t> &buffer, MessageParsing **message)
{
    auto errMessage = new ErrMessage();
    bool success = errMessage->deserialize(buffer, protocol);
    if (success) {
        *message = errMessage;
    }
    return success;
}
bool ConnectionClient::HandleIncomingPingMessage(const vector<uint8_t> &buffer, MessageParsing **message)
{
    auto pingMessage = new PingMessage();; // new message

    bool success = pingMessage->deserialize(buffer, protocol);
    if (success) {
        *message = pingMessage;
    }else
    {
        delete pingMessage; // memory safety
    }
    return success;
}

void ConnectionClient::Transmit(const vector<uint8_t> &buffer)
{
    if (protocol == Protocol::TCP) {
        send(clientSocket, buffer.data(), buffer.size(), 0);
    } else if (protocol == Protocol::UDP) {
        sendto(clientSocket, buffer.data(), buffer.size(), 0, (struct sockaddr *) &dstAddress, sizeof(dstAddress));
    }
}


void create_sockaddr_in(sockaddr_in &sockaddr, const string &address, uint16_t port)
{
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);

    // Attempt to parse address as an IP address
    if (inet_pton(AF_INET, address.c_str(), &sockaddr.sin_addr) == 1)
        return;

    // If it's not a valid IP address, try DNS resolution
    addrinfo hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo *result;
    int err = getaddrinfo(address.c_str(), nullptr, &hints, &result);
    if (err != 0) {
        throw std::runtime_error("ERROR: getaddrinfo failed: " + std::string(gai_strerror(err)));
    }

    for (addrinfo *p = result; p != nullptr; p = p->ai_next) {
        sockaddr.sin_addr = ((sockaddr_in *) p->ai_addr)->sin_addr;
    }

    freeaddrinfo(result);
}

ConnectionClient *createTCPConnection(const string &address, uint16_t port)
{
    sockaddr_in sockaddr = {};
    create_sockaddr_in(sockaddr, address, port);
    return new ConnectionClient(sockaddr);
}

ConnectionClient *createUDPConnection(const string &address, uint16_t port, uint16_t retransmitTimeout, uint8_t retransmitCount)
{
    sockaddr_in sockaddr = {};
    create_sockaddr_in(sockaddr, address, port);
    return new ConnectionClient(sockaddr, retransmitTimeout, retransmitCount);
}

