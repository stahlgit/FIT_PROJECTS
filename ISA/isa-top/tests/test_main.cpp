#include <gtest/gtest.h>
#include "../src/include/utils.h"


TEST(ParseArgumentsTest, ValidArguments) {
    char* argv[] = {const_cast<char*>("program"), const_cast<char*>("-i"), const_cast<char*>("lo"), const_cast<char*>("-s"), const_cast<char*>("b"), const_cast<char*>("-t"), const_cast<char*>("5")};
    int argc = sizeof(argv) / sizeof(char*);

    Config config = parse_arguments(argc, argv);
    EXPECT_EQ(config.interface, "lo");
    EXPECT_EQ(config.sort_option, 'b');
    EXPECT_EQ(config.interval, 5);
}

TEST(ParseArgumentsTest, MissingInterface) {
    char* argv[] = {const_cast<char*>("program"), const_cast<char*>("-s"), const_cast<char*>("b"), nullptr};
    int argc = 3;

    EXPECT_THROW(parse_arguments(argc, argv), std::invalid_argument);
}

TEST(ParseArgumentsTest, InvalidSortOption) {
    char* argv[] = {const_cast<char*>("program"), const_cast<char*>("-i"), const_cast<char*>("eth0"), const_cast<char*>("-s"), const_cast<char*>("x")};
    int argc = sizeof(argv) / sizeof(char*);

    EXPECT_THROW(parse_arguments(argc, argv), std::invalid_argument);
} 

TEST(ParseArgumentsTest, InvalidInterval) {
    char* argv[] = {const_cast<char*>("program"), const_cast<char*>("-i"), const_cast<char*>("eth0"), const_cast<char*>("-t"), const_cast<char*>("-1")};
    int argc = sizeof(argv) / sizeof(char*);

    EXPECT_THROW(parse_arguments(argc, argv), std::invalid_argument);
}

TEST(ParseArgumentsTest, NoArguments) {
    char* argv[] = {const_cast<char*>("program")};
    int argc = sizeof(argv) / sizeof(char*);

    EXPECT_THROW(parse_arguments(argc, argv), std::invalid_argument);
}

