//
// Created by peter on 20.4.2025.
//
// ArgumentParserTest.cpp
#include "gtest/gtest.h"
#include "../include/ArgumentParser.hpp"

TEST(ArgumentParserTest, ValidArguments) {
    const char* argv[] = {"program", "-t", "udp", "-s", "localhost"};
    int argc = sizeof(argv)/sizeof(argv[0]);
    ArgumentParser parser(argc, const_cast<char**>(argv));
    ProgramOptions opts = parser.parse();
    EXPECT_EQ(opts.transport, "udp");
    EXPECT_EQ(opts.server, "localhost");
    EXPECT_EQ(opts.port, 4567); // default
}

TEST(ArgumentParserTest, MissingRequiredArgs) {
    const char* argv[] = {"program", "-p", "1234"};
    int argc = sizeof(argv)/sizeof(argv[0]);
    ArgumentParser parser(argc, const_cast<char**>(argv));
    EXPECT_THROW(parser.parse(), std::invalid_argument);
}

TEST(ArgumentParserTest, MissingRequiredArgsServer) {
    const char* argv[] = {"program", "-t", "udp"};
    int argc = sizeof(argv)/sizeof(argv[0]);
    ArgumentParser parser(argc, const_cast<char**>(argv));
    EXPECT_THROW(parser.parse(), std::invalid_argument);
}


TEST(ArgumentParserTest, MissingRequiredArgsTransport) {
    const char* argv[] = {"program", "-s", "localhost"};
    int argc = sizeof(argv)/sizeof(argv[0]);
    ArgumentParser parser(argc, const_cast<char**>(argv));
    EXPECT_THROW(parser.parse(), std::invalid_argument);
}


TEST(ArgumentParserTest, WrongTransport) {
    const char* argv[] = {"program", "-t", "icmp", "-s", "localhost"};
    int argc = sizeof(argv)/sizeof(argv[0]);
    ArgumentParser parser(argc, const_cast<char**>(argv));
    EXPECT_THROW(parser.parse(), std::invalid_argument);
}
