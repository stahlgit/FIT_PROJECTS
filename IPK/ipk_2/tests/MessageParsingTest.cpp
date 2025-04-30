#include "gtest/gtest.h"
#include "../include/MessageParsing.hpp"
#include <stdexcept>

// Helper function to create UDP headers
std::vector<uint8_t> createUdpHeader(MessageType type, uint16_t messageId) {
    return {static_cast<uint8_t>(type), 
           static_cast<uint8_t>(messageId >> 8), 
           static_cast<uint8_t>(messageId & 0xFF)};
}



TEST(ConfirmMessageTest, UdpSerialization) {
    ConfirmMessage msg(0x1234);
    std::vector<uint8_t> buffer;
    msg.serialize(buffer, Protocol::UDP);
    ASSERT_EQ(buffer.size(), 3);
    EXPECT_EQ(buffer[0], static_cast<uint8_t>(MessageType::CONFIRM));
    EXPECT_EQ(buffer[1], 0x12);
    EXPECT_EQ(buffer[2], 0x34);
}

TEST(ConfirmMessageTest, TcpSerializeThrows) {
    ConfirmMessage msg;
    std::vector<uint8_t> buffer;
    EXPECT_THROW(msg.serialize(buffer, Protocol::TCP), std::runtime_error);
}

TEST(ReplyMessageTest, TcpSerialization) {
    ReplyMessage msg;
    msg.result = true;
    msg.messageContents = "Success";
    std::vector<uint8_t> buffer;
    msg.serialize(buffer, Protocol::TCP);
    EXPECT_EQ(std::string(buffer.begin(), buffer.end()), "REPLY OK IS Success\r\n");
}

TEST(ReplyMessageTest, UdpSerialization) {
    ReplyMessage msg;
    msg.messageId = 0x1234;
    msg.result = true;
    msg.refMessageId = 0x5678;
    msg.messageContents = "Hi";
    std::vector<uint8_t> buffer;
    msg.serialize(buffer, Protocol::UDP);
    
    std::vector<uint8_t> expected = createUdpHeader(MessageType::REPLY, 0x1234);
    expected.push_back(1);  // Result
    expected.push_back(0x56);  // refMessageId high
    expected.push_back(0x78);  // refMessageId low
    appendString(expected, "Hi");
    expected.push_back('\0');
    
    EXPECT_EQ(buffer, expected);
}

TEST(ReplyMessageTest, UdpDeserialization) {
    std::vector<uint8_t> buffer = createUdpHeader(MessageType::REPLY, 0x1234);
    buffer.insert(buffer.end(), {0x01, 0x56, 0x78});  // Result + refMessageId
    appendString(buffer, "Hello");
    buffer.push_back('\0');

    ReplyMessage msg;
    EXPECT_TRUE(msg.deserialize(buffer, Protocol::UDP));
    EXPECT_EQ(msg.messageId, 0x1234);
    EXPECT_TRUE(msg.result);
    EXPECT_EQ(msg.refMessageId, 0x5678);
    EXPECT_EQ(msg.messageContents, "Hello");
}

TEST(AuthMessageTest, UdpSerialization) {
    AuthMessage msg("user", "name", "secret");
    msg.messageId = 0x1234;
    std::vector<uint8_t> buffer;
    msg.serialize(buffer, Protocol::UDP);
    
    std::vector<uint8_t> expected = createUdpHeader(MessageType::AUTH, 0x1234);
    appendString(expected, "user");
    expected.push_back('\0');
    appendString(expected, "name");
    expected.push_back('\0');
    appendString(expected, "secret");
    expected.push_back('\0');
    
    EXPECT_EQ(buffer, expected);
}

TEST(JoinMessageTest, TcpSerialization) {
    JoinMessage msg("channel1", "user");
    std::vector<uint8_t> buffer;
    msg.serialize(buffer, Protocol::TCP);
    EXPECT_EQ(std::string(buffer.begin(), buffer.end()), "JOIN channel1 AS user\r\n");
}

TEST(MsgMessageTest, UdpDeserialization) {
    std::vector<uint8_t> buffer = createUdpHeader(MessageType::MSG, 0x1234);
    appendString(buffer, "sender");
    buffer.push_back('\0');
    appendString(buffer, "Hello world");
    buffer.push_back('\0');
    
    MsgMessage msg;
    EXPECT_TRUE(msg.deserialize(buffer, Protocol::UDP));
    EXPECT_EQ(msg.displayName, "sender");
    EXPECT_EQ(msg.messageContents, "Hello world");
}

TEST(ErrMessageTest, TcpDeserialization) {
    std::string errMsg = "ERR FROM server IS Critical error\r\n";
    std::vector<uint8_t> buffer(errMsg.begin(), errMsg.end());
    
    ErrMessage msg;
    EXPECT_TRUE(msg.deserialize(buffer, Protocol::TCP));
    EXPECT_EQ(msg.displayName, "server");
    EXPECT_EQ(msg.messageContents, "Critical error");
}

TEST(ByeMessageTest, TcpSerialization) {
    ByeMessage msg;
    std::vector<uint8_t> buffer;
    msg.serialize(buffer, Protocol::TCP);
    EXPECT_EQ(std::string(buffer.begin(), buffer.end()), "BYE\r\n");
}

TEST(ByeMessageTest, UdpDeserialization) {
    std::vector<uint8_t> buffer = createUdpHeader(MessageType::BYE, 0x1234);
    ByeMessage msg;
    EXPECT_TRUE(msg.deserialize(buffer, Protocol::UDP));
    EXPECT_EQ(msg.messageId, 0x1234);
}

TEST(HelperFunctionsTest, AppendString) {
    std::vector<uint8_t> buffer;
    appendString(buffer, "test");
    EXPECT_EQ(buffer, std::vector<uint8_t>({'t', 'e', 's', 't'}));
}

TEST(HelperFunctionsTest, ConfirmMessageRemoval) {
    std::vector<MessageToConfirm> messages = {
        {new ConfirmMessage(0x1234), 0, 3},
        {new ConfirmMessage(0x5678), 0, 2}
    };
    
    confirmMessage(messages, 0x1234);
    EXPECT_EQ(messages.size(), 1);
    EXPECT_EQ(messages[0].message->messageId, 0x5678);
}

TEST(EdgeCasesTest, ShortUdpBuffer) {
    std::vector<uint8_t> buffer = {0x01};  // Too short
    ReplyMessage msg;
    EXPECT_FALSE(msg.deserialize(buffer, Protocol::UDP));
}

TEST(EdgeCasesTest, InvalidMessageType) {
    std::vector<uint8_t> buffer = createUdpHeader(MessageType::CONFIRM, 0x1234);
    ReplyMessage msg;  // Expecting REPLY type
    EXPECT_FALSE(msg.deserialize(buffer, Protocol::UDP));
}

TEST(EdgeCasesTest, MissingNullTerminator) {
    std::vector<uint8_t> buffer = createUdpHeader(MessageType::AUTH, 0x1234);
    appendString(buffer, "user");  // Missing null terminator
    AuthMessage msg;
    EXPECT_FALSE(msg.deserialize(buffer, Protocol::UDP));
}