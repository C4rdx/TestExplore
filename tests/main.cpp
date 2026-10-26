#include <TestExplorer/TestRunner.hpp>
#include <TestExplorer/TestRegistry.hpp>
#include <TestExplorer/ConsoleReporter.hpp>

#include "test_runner.hpp"
#include "test_skipping.hpp"
#include "test_stability.hpp"
#include "test_assertions.hpp"
#include "test_exceptions.hpp"
#include "test_registration.hpp"

#include <iostream>

int main()
{
    testexplorer::TestRegistry registry;
    registerFrameworkTests(registry);

    testexplorer::TestRegistry exceptionRegistry;
    registerExceptionTests(exceptionRegistry);

    testexplorer::ConsoleReporter reporter(std::cout);
    testexplorer::TestRunner runner(&reporter);

    // Assertion tests
    const auto results =
        runner.runAll(registry);

    if (!verifyAssertionResults(results))
    {
        return 1;
    }

    if (!verifyAssertionFailures(results))
    {
        return 1;
    }

    // Runner tests
    if (!runRunnerTests(
            runner,
            registry))
    {
        return 1;
    }

    // Exception tests
    if (!runExceptionTests(
            runner,
            exceptionRegistry))
    {
        return 1;
    }

    // Stability tests
    if (!runStabilityTests(
            runner,
            registry,
            exceptionRegistry))
    {
        return 1;
    }

    if (!runSkippingTests(runner))
    {
        return 1;
    }

    // Self-test summary
    std::cout
        << "\n=== Framework Self-Tests Passed ===\n";

    return 0;
}