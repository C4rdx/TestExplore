#ifndef ASSERTIONS_HPP
#define ASSERTIONS_HPP

#include <TestExplorer/TestContext.hpp>
#include <TestExplorer/TestFailure.hpp>
#include <TestExplorer/CurrentTestContext.hpp>

#include <string>
#include <sstream>
#include <source_location>

namespace testexplorer
{
    template <typename T>
    std::string formatValue(const T &value)
    {
        if constexpr (requires(std::ostream &stream) { stream << value; })
        {
            std::ostringstream stream;
            stream << value;
            return stream.str();
        }
        else
        {
            return "<unprintable>";
        }
    }

    inline void expectTrue(
        TestContext &context,
        bool condition,
        std::source_location location =
            std::source_location::current())
    {
        if (!condition)
        {
            context.addFailure(
                TestFailure(
                    "Expected condition to be true",
                    location));
        }
    }

    inline void expectFalse(
        TestContext &context,
        bool condition,
        std::source_location location =
            std::source_location::current())
    {
        if (condition)
        {
            context.addFailure(
                TestFailure(
                    "Expected condition to be false",
                    location));
        }
    }

    template <typename T, typename U>
    void expectEqual(
        TestContext &context,
        const T &actual,
        const U &expected,
        std::source_location location =
            std::source_location::current())
    {
        if (!(actual == expected))
        {
            std::ostringstream message;

            message
                << "Expected values to be equal\n"
                << "Expected: " << formatValue(expected) << '\n'
                << "Actual:   " << formatValue(actual);

            context.addFailure(
                TestFailure(
                    message.str(),
                    location));
        }
    }

    template <typename T, typename U>
    void expectNotEqual(
        TestContext &context,
        const T &actual,
        const U &expected,
        std::source_location location =
            std::source_location::current())
    {
        if (actual == expected)
        {
            std::ostringstream message;

            message
                << "Expected values to be different\n"
                << "Value: " << formatValue(actual);

            context.addFailure(
                TestFailure(
                    message.str(),
                    location));
        }
    }
}

#define EXPECT_TRUE(condition)                     \
    ::testexplorer::expectTrue(                    \
        ::testexplorer::CurrentTestContext::get(), \
        (condition))

#define EXPECT_FALSE(condition)                    \
    ::testexplorer::expectFalse(                   \
        ::testexplorer::CurrentTestContext::get(), \
        (condition))

#define EXPECT_EQ(actual, expected)                \
    ::testexplorer::expectEqual(                   \
        ::testexplorer::CurrentTestContext::get(), \
        (actual),                                  \
        (expected))

#define EXPECT_NE(actual, expected)                \
    ::testexplorer::expectNotEqual(                \
        ::testexplorer::CurrentTestContext::get(), \
        (actual),                                  \
        (expected))

#endif