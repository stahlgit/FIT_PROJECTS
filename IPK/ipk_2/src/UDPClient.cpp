#include "UDPClient.hpp"

#include <ctime>
#include <iostream>
#include <stdexcept>
#include "userInput.hpp"

using namespace std;

int calculatePollTimeout(const vector<MessageToConfirm>& messagesToConfirm)
{
    if (messagesToConfirm.empty())
    {
        return -1; // No messages to confirm, poll indefinitely.
    }
    time_t currentTime = time(nullptr);
    // Get the retransmit time of the earliest message.
    auto nextTime = messagesToConfirm[0].scheduledRetransmitTimestamp;
    // Calculate the timeout, ensuring it's not negative.
    return max(0, static_cast<int>(nextTime - currentTime));

}

void handlePollTimeout(ConnectionClient* connection, vector<MessageToConfirm>& messagesToConfirm) {
    if (!messagesToConfirm.empty()) {
        time_t currentTime = time(nullptr);
        MessageToConfirm& messageToConfirm = messagesToConfirm[0];  // Get a reference to the earliest message to confirm.
        connection->SendMessage(*messageToConfirm.message); // Re-send
        messageToConfirm.scheduledRetransmitTimestamp = currentTime + connection->GetRetransmitTimeout(); // Update the next retransmit time
        messageToConfirm.retransmitCount--;
    }
}

void handleIncomingUdpMessage(ConnectionClient* connection, MessageParsing* message, vector<MessageToConfirm>& messagesToConfirm)
{
    // CONFIRM
    if (auto confirm = dynamic_cast<ConfirmMessage*>(message)){
        confirmMessage(messagesToConfirm, confirm->messageId);
    }
    // REPLY
    else if (auto reply = dynamic_cast<ReplyMessage*>(message)){
        cout << (reply->result ? "Action Success: " : "Failure: ") << reply->messageContents << endl;
        connection->SendConfirm(reply->messageId);
    }
    // MSG
    else if (auto msg = dynamic_cast<MsgMessage*>(message)){
        cout << msg->displayName << ": " << msg->messageContents << endl;
        connection->SendConfirm(msg->messageId);
    }

    // PING
    else if (auto ping = dynamic_cast<PingMessage*>(message))
    {
        connection->SendConfirm(ping->messageId);
    }
    // BYE
    else if (auto bye = dynamic_cast<ByeMessage*>(message))
    {
        cout << "Connection closed due to inactivity" << endl;
        connection->SendConfirm(bye->messageId);
    }
    // ERR
    else if (auto err = dynamic_cast<ErrMessage*>(message)){
        cerr << "ERR FROM " << err->displayName << ": " << err->messageContents << endl;
        connection->SendConfirm(err->messageId);
        if (connection->state == ConnectionState::Open || connection->state == ConnectionState::Auth) {
            connection->state = ConnectionState::End;
            connection->SendBye();
            MessageToConfirm messageToConfirm = {
                connection->SendBye(),
                time(nullptr) + connection->GetRetransmitTimeout(),
                connection->GetRetransmitCount()
        };
            messagesToConfirm.push_back(messageToConfirm);
        }
    }
}


int handleUdpPollFds(ConnectionClient* connection, struct pollfd* pollFds, vector<MessageToConfirm>& messagesToConfirm) {
    // Check for incoming data on the connection socket.

    if (pollFds[0].revents & POLLIN) {
        MessageParsing* message = nullptr;
        if (!connection->HandleIncomingData(&message)) {
            cerr << "ERROR: invalid message arrived THIS " << endl;

            connection->state = ConnectionState::Error;
            return 2; // Continue to the next iteration of the loop
        }
        if (message != nullptr)
        {
            handleIncomingUdpMessage(connection, message, messagesToConfirm);
            delete message;
        }
    }
    // Check for incoming data from standard input (user input).
    if (pollFds[1].revents & POLLIN) {
        handleInput(connection, messagesToConfirm);
    }
    // Check for hang-up on stdin or errors/incoming signals on the signal file descriptor.
    if (pollFds[1].revents & POLLHUP || pollFds[2].revents & (POLLIN | POLLERR)) {
        if (connection->state == ConnectionState::Start) {
            connection->state = ConnectionState::End;
            return 1; // break
        }
        // Send a "bye" message and add it to the confirmation list.
        auto bye = connection->SendBye();
        messagesToConfirm.push_back({bye, time(nullptr) + connection->GetRetransmitTimeout(), connection->GetRetransmitCount()});
        connection->state = ConnectionState::End;

        return 1; // exit loop
    }
    // Check for incoming signals or errors on the signal file descriptor.
    if (pollFds[2].revents & POLLIN || pollFds[2].revents & POLLERR) {
        if (connection->state == ConnectionState::Start) {
            connection->state = ConnectionState::End;
            return 1;
        }
        connection->SendBye();
        MessageToConfirm messageToConfirm = {
            connection->SendBye(),
            time(nullptr) + connection->GetRetransmitTimeout(),
            connection->GetRetransmitCount()
    };
        messagesToConfirm.push_back(messageToConfirm);
        return 1;
    }
    return 0;
}


void handleUDP(ConnectionClient *connection, pollfd *pollFds, nfds_t numberOfFds) {
    vector<MessageToConfirm> messagesToConfirm;

    while (true) {
        // Calculate the poll timeout based on the earliest message to retransmit.
        int timeout = calculatePollTimeout(messagesToConfirm);
        // Wait for events on the monitored file descriptors or until the timeout expires.
        int changedCount = poll(pollFds, numberOfFds, timeout);
        // Sort the messages to confirm by their retransmit timestamp.
        if (changedCount == -1) {
            throw runtime_error("Error polling");
        }
        // Sort the messages to confirm by their retransmit timestamp.
        if (timeout != -1) {
            sortMessagesToConfirm(messagesToConfirm);
        }
        // Handle a poll timeout (no events occurred within the timeout).
        if (changedCount == 0) {
            // Retransmit the earliest unconfirmed message.
            handlePollTimeout(connection, messagesToConfirm);
        } else {
            int result = handleUdpPollFds(connection, pollFds, messagesToConfirm);
            if (result == 1){
                break;
            }
            if (result == 2){
                continue;
            }
        }
        // Check if there are any messages that have exceeded the maximum number of retransmits.
        if (areThereFailedMessages(messagesToConfirm)) {
            connection->state = ConnectionState::Error;
            break;
        }
        // Check the overall connection state.
        if (connection->state == ConnectionState::Error) {
            connection->SendBye();
            MessageToConfirm messageToConfirm = {
                    connection->SendBye(),
                    time(nullptr) + connection->GetRetransmitTimeout(),
                    connection->GetRetransmitCount()
            };
            messagesToConfirm.push_back(messageToConfirm);
            connection->state = ConnectionState::End;
            break;
        } else if (connection->state == ConnectionState::End && messagesToConfirm.empty()) {
            break;
        }
    }
    // Clean up the connection object.
    delete connection;
}
