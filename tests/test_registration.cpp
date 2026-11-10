#include "test_registration.hpp"

#include <TestExplorer/TestCase.hpp>
#include <TestExplorer/Assertions.hpp>
#include <TestExplorer/TestContext.hpp>

#include <string>
#include <stdexcept>

using namespace testexplorer;

namespace
{
    struct NonStreamableValue
    {
        int value;

        friend bool operator==(
            const NonStreamableValue &,
            const NonStreamableValue &) = default;
    };
}

void registerFrameworkTests(
    TestRegistry &registry)
{
    // Assertion tests
    registry.registerTest(
        TestCase(
            "assertions.passing",
            "Passing Assertions",
            [](TestContext &)
            {
                EXPECT_TRUE(true);
                EXPECT_FALSE(false);

                EXPECT_EQ(10, 10);
                EXPECT_NE(10, 20);

                const NonStreamableValue first{1};
                const NonStreamableValue same{1};
                const NonStreamableValue different{2};

                EXPECT_EQ(first, same);
                EXPECT_NE(first, different);
                EXPECT_EQ(formatValue(first), "<unprintable>");

                TestContext equalContext;
                expectEqual(equalContext, first, different);
                EXPECT_EQ(equalContext.failures().size(), 1U);
                EXPECT_TRUE(
                    equalContext.failures().front().message().find("<unprintable>")
                    != std::string::npos);

                TestContext notEqualContext;
                expectNotEqual(notEqualContext, first, same);
                EXPECT_EQ(notEqualContext.failures().size(), 1U);
                EXPECT_TRUE(
                    notEqualContext.failures().front().message().find("<unprintable>")
                    != std::string::npos);
            }));

    registry.registerTest(
        TestCase(
            "assertions.failing",
            "Failing Assertions",
            [](TestContext &)
            {
                EXPECT_TRUE(false);
                EXPECT_FALSE(true);

                EXPECT_EQ(10, 20);
                EXPECT_NE(10, 10);
            }));

    registry.registerTest(
        TestCase(
            "assertions.strings",
            "String Assertions",
            [](TestContext &)
            {
                EXPECT_EQ(
                    std::string("hello"),
                    std::string("hello"));

                EXPECT_NE(
                    std::string("hello"),
                    std::string("world"));
            }));

    registry.registerTest(
        TestCase(
            "assertions.multiple_failures",
            "Multiple Failures",
            [](TestContext &)
            {
                EXPECT_EQ(1, 2);
                EXPECT_EQ(3, 4);
                EXPECT_TRUE(false);
            }));

    // Filtering tests
    registry.registerTest(
        TestCase(
            "math.addition",
            "Math Addition",
            [](TestContext &)
            {
                EXPECT_EQ(2 + 2, 4);
            }));

    registry.registerTest(
        TestCase(
            "math.subtraction",
            "Math Subtraction",
            [](TestContext &)
            {
                EXPECT_EQ(5 - 3, 2);
            }));

    registry.registerTest(
        TestCase(
            "string.compare",
            "String Compare",
            [](TestContext &)
            {
                EXPECT_EQ(
                    std::string("hello"),
                    std::string("hello"));
            }));
}

void registerExceptionTests(
    TestRegistry &registry)
{
    registry.registerTest(
        TestCase(
            "exceptions.runtime_error",
            "Runtime Error",
            [](TestContext &)
            {
                throw std::runtime_error("runtime error");
            }));

    registry.registerTest(
        TestCase(
            "exceptions.unknown",
            "Unknown Exception",
            [](TestContext &)
            {
                throw 42;
            }));

    registry.registerTest(
        TestCase(
            "exceptions.after_error",
            "Test After Error",
            [](TestContext &)
            {
                EXPECT_TRUE(true);
            }));
}