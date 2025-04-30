#include <regex>
#include <iostream>
#include "../include/userInput.hpp"
#include "../include/ConnectionClient.hpp"

using namespace std;
// REGEX Expression (better to have them here, for Encapsulation, Reduced Coupling, Header Pollution, ... )
// Regular expressions for matching command prefixes.
static regex authCommandPrefixRegex(R"(\/[aA][uU][tT][hH]\b.*)");
static regex joinCommandPrefixRegex(R"(\/[jJ][oO][iI][nN]\b.*)");
static regex renameCommandPrefixRegex(R"(\/[rR][eE][nN][aA][mM][eE]\b.*)");
// Regular expressions for matching the full command syntax and capturing arguments.
static regex authCommandRegex(R"(\/[aA][uU][tT][hH]\s+([^\s]+)\s+([^\s]+)\s+([^\s]+)\s*)");
static regex joinCommandRegex(R"(\/[jJ][oO][iI][nN]\s+([^\s]+\s*))");
static regex renameCommandRegex(R"(\/[rR][eE][nN][aA][mM][eE]\s+([^\s]+))");
static regex helpCommandRegex(R"(\/[hH][eE][lL][pP]\s*)");
static regex exitCommandRegex(R"(\/[eE][xX][iI][tT]\s*)");

void handleInput(ConnectionClient *connection, vector<MessageToConfirm> &messagesToConfirm) {
    string line;
    getline(cin, line);
    smatch match;

    // Check if the input starts with the /auth command prefix.
    if (regex_match(line, authCommandPrefixRegex))
    {
        // Attempt to match the full /auth command syntax.
        bool authMatched = regex_match(line, match, authCommandRegex);
        if (authMatched) {
            handleAuthCommandInput(connection, match, messagesToConfirm);
            return;
        }
        cout << "/auth <username> <secret> <display name> - authenticate with the server" << endl;
        return;
    }
    // Check if the input starts with the /join command prefix.
    if (regex_match(line, joinCommandPrefixRegex))
    {
        // Attempt to match the full /join command syntax.
        bool joinMatched = regex_match(line, match, joinCommandRegex);
        if (joinMatched) {
            handleJoinCommandInput(connection, match, messagesToConfirm);
            return;
        }
        cout << "/join <channel id> - join a channel" << endl;
        return;
    }
    // Check if the input starts with the /rename command prefix.
    if (regex_match(line, renameCommandPrefixRegex))
    {
        // Attempt to match the full /rename command syntax.
        bool renameMatched = regex_match(line, match, renameCommandRegex);
        if (renameMatched) {
            handleRenameCommandInput(connection, match);
            return;
        }
        cout << "Unsupported name format. Use: /rename <new_display_name>" << endl;
        return ;
    }
    // Check if the input matches the /exit command.
    bool exitMatched = regex_match(line, match, exitCommandRegex);
    if (exitMatched) {
        handleExitCommandInput(connection, messagesToConfirm);
        return;
    }
    // Check if the input matches the /help command.
    bool helpMatched = regex_match(line, match, helpCommandRegex);
    if (helpMatched) {
        handleHelpCommandInput();
        return;
    }
    // If the input starts with a '/' but doesn't match any known command, inform the user.
    if (!line.empty() && line[0] == '/') {
        cout << "Unknown command. Type /help for the list of commands." << endl;
        return;
    }

    handleMessageInput(connection, line, messagesToConfirm);
}

void handleAuthCommandInput(ConnectionClient *connection, const smatch &match, vector<MessageToConfirm> &messagesToConfirm) {
    string username = match[1];
    string secret = match[2];
    string displayName = match[3];
    // Send the authentication message to the server.
    AuthMessage *message = connection->SendAuth(username, displayName, secret);
    if (message == nullptr) {
        return;
    }
    // If the connection is using UDP, add the message to the confirmation list.
    if (connection->GetProtocol() == Protocol::UDP) {
        MessageToConfirm messageToConfirm = {
                message,
                time(nullptr) + connection->GetRetransmitTimeout(),
                connection->GetRetransmitCount()
        };
        messagesToConfirm.push_back(messageToConfirm);
    }
}

void handleJoinCommandInput(ConnectionClient *connection, const smatch &match, vector<MessageToConfirm> &messagesToConfirm) {
    string channelId = match[1];
    // Send the join channel message to the server.
    JoinMessage *message = connection->SendJoin(channelId);
    if (message == nullptr) {
        return;
    }
    // If the connection is using UDP, add the message to the confirmation list.
    if (connection->GetProtocol() == Protocol::UDP) {
        MessageToConfirm messageToConfirm = {
                message,
                time(nullptr) + connection->GetRetransmitTimeout(),
                connection->GetRetransmitCount()
        };
        messagesToConfirm.push_back(messageToConfirm);
    }
}

void handleMessageInput(ConnectionClient *connection, const string &message, vector<MessageToConfirm> &messagesToConfirm) {
    // Send the regular chat message to the server.
    MsgMessage *msgMessage = connection->SendMsg(message);
    if (msgMessage == nullptr) { // If sending fails, exit the function.
        return;
    }
    // If the connection is using UDP, add the message to the confirmation list.
    if (connection->GetProtocol() == Protocol::UDP) {
        MessageToConfirm messageToConfirm = {
                msgMessage,
                time(nullptr) + connection->GetRetransmitTimeout(),
                connection->GetRetransmitCount()
        };
        messagesToConfirm.push_back(messageToConfirm);
    }
}

void handleRenameCommandInput(ConnectionClient *connection, const smatch &match) {
    string newName = match[1];
    // Change users Display name
    connection->Rename(newName);
}

void handleExitCommandInput(ConnectionClient *connection, vector<MessageToConfirm> &messagesToConfirm) {
    if (connection->state == ConnectionState::Start) {
        connection->state = ConnectionState::End;
        return;
    }
    // Send the "bye" message to the server.
    ByeMessage *byeMessage = connection->SendBye();
    if (byeMessage == nullptr) {
        return;
    }
    // If the connection is using UDP, add the "bye" message to the confirmation list.
    if (connection->GetProtocol() == Protocol::UDP) {
        MessageToConfirm messageToConfirm = {
                byeMessage,
                time(nullptr) + connection->GetRetransmitTimeout(),
                connection->GetRetransmitCount()
        };
        messagesToConfirm.push_back(messageToConfirm);
    }
}

void handleHelpCommandInput() {
    cout << "Available commands:" << endl;
    cout << "/auth <username> <secret> <display name> - authenticate with the server" << endl;
    cout << "/join <channel id> - join a channel" << endl;
    cout << "/rename <new display name> - change your display name" << endl;
    cout << "/exit - exit the program and close the connection" << endl;
    cout << "/help - show this help" << endl;
}
