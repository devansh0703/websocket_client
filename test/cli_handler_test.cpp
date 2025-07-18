#include <gtest/gtest.h>
#include "cli_handler.hpp"

namespace websocket_client {
namespace test {

TEST(CLIHandlerTest, DefaultValues) {
    CLIHandler cli;
    const char* argv[] = {"program"};
    ASSERT_TRUE(cli.parse(1, const_cast<char**>(argv)));
    
    EXPECT_EQ(cli.getHost(), "echo.websocket.org");
    EXPECT_EQ(cli.getPort(), "443");
    EXPECT_EQ(cli.getTarget(), "/");
    EXPECT_TRUE(cli.isSecure());
}

TEST(CLIHandlerTest, CustomValues) {
    CLIHandler cli;
    const char* argv[] = {
        "program",
        "--host", "localhost",
        "--port", "8080",
        "--target", "/echo",
        "--no-secure"
    };
    ASSERT_TRUE(cli.parse(8, const_cast<char**>(argv)));
    
    EXPECT_EQ(cli.getHost(), "localhost");
    EXPECT_EQ(cli.getPort(), "8080");
    EXPECT_EQ(cli.getTarget(), "/echo");
    EXPECT_FALSE(cli.isSecure());
}

} // namespace test
} // namespace websocket_client
