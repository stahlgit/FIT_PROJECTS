#include "TCPClient.hpp"
#include <iostream>
using namespace std;

void processIncomingMessage(ConnectionClient *connection)
{
    MessageParsing *message; // For storing the parsed message
    bool parseResult = connection->HandleIncomingData(&message);
    if (!parseResult) {
        cerr << "ERROR: invalid message arrived" << endl;
        connection->state = ConnectionState::Error;
        return;
    }
    // Check the type of the received message
    // REPLY
    if (auto reply = dynamic_cast<ReplyMessage *>(message)) {
        if (reply->result) {
            cerr << "Action Success: " << reply->messageContents << endl;
        } else {
            cerr << "Failure: " << reply->messageContents << endl;
        }
    }
    // CHAT MSG
    else if (auto msg = dynamic_cast<MsgMessage *>(message))
    {
        cout << msg->displayName << ": " << msg->messageContents << endl;
    }
    //ERR
    else if (auto err = dynamic_cast<ErrMessage *>(message))
    {
        cerr << "ERR FROM " << err->displayName << ": " << err->messageContents << endl;
        // If the connection is currently open or in the authentication state,
        // set the state to End and send a "bye" message to the server.
        if (connection->state == ConnectionState::Open || connection->state == ConnectionState::Auth) {
            connection->state = ConnectionState::End;
            connection->SendBye();
        }
    }
    // BYE
    else if (auto bye = dynamic_cast<ByeMessage *>(message)) // parameter used only to check if it's not nullptr
    {
        connection->state = ConnectionState::End;
    }
    // Free the memory allocated for the parsed message.
    delete message;
}


void handleTcpPollFds(struct pollfd *pollFds, ConnectionClient *connection) {
    vector<MessageToConfirm> messagesToConfirm; // ...
    // Check for incoming data on the connection socket.
    if (pollFds[0].revents & POLLIN) {
        processIncomingMessage(connection);
    }
    // Check for incoming data from standard input (user input).
    if (pollFds[1].revents & POLLIN) {
        // Call the handleInput function to process user input.
        handleInput(connection, messagesToConfirm);
    }
    // Check for a hang-up event on standard input (e.g., if the input stream is closed).
    if (pollFds[1].revents & POLLHUP) {
        // Hangup
        connection->SendBye();
        return;
    }
    // Check for events on the signal file descriptor (e.g., SIGINT).
    if (pollFds[2].revents & POLLIN) {
        // If the connection is in the initial Start state, simply transition to the End state.
        if (connection->state == ConnectionState::Start) {
            connection->state = ConnectionState::End;
            return;
        }
        // Otherwise, send a "bye" message to the server and transition to the End state.
        connection->SendBye();
        return;
    }
}

void handleTCP(ConnectionClient *connection, pollfd *pollFds, nfds_t numberOfFds) {
    while (true) {
        // Wait for events on the monitored file descriptors. The -1 timeout means poll will block indefinitely.
        int changedCount = poll(pollFds, numberOfFds, -1);
        if (changedCount == -1) {
            // If poll returns -1, an error occurred. Throw a runtime exception.
            throw runtime_error("ERROR: Polling");
        } else {
            // If poll returns a positive value, it indicates the number of file descriptors with events.
            handleTcpPollFds(pollFds, connection);
        }
        // Check the current state of the connection.
        if (connection->state == ConnectionState::Error) {
            // If the connection is in an error state, send a "bye" message and break out of the loop.
            connection->SendBye();
            break;
        } else if (connection->state == ConnectionState::End) {
            break;
        }
    }
    // After the loop finishes (connection ended or errored), clean up the connection object.
    delete connection;
}