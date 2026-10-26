#include <TestExplorer/TestCase.hpp>
#include <TestExplorer/TestResult.hpp>
#include <TestExplorer/TestRunner.hpp>
#include <TestExplorer/TestContext.hpp>
#include <TestExplorer/TestRegistry.hpp>
#include <TestExplorer/CurrentTestContext.hpp>

#include <string>
#include <iostream>
#include <stdexcept>

using namespace testexplorer;

bool runSkippingTests(TestRunner &runner)
{
    TestCase skippedTest(
        "skipping.with_reason",
        "Skipped Test With Reason",
        [](TestContext &context)
        {
            context.skip("Dependency is unavailable");
        });

    const TestResult skipped = runner.run(skippedTest);

    if (skipped.status() != TestStatus::Skipped ||
        skipped.skipReason() != "Dependency is unavailable" ||
        !skipped.errorMessage().empty() ||
        !skipped.failures().empty())
    {
        std::cerr
            << "Self-test error: skip result was not recorded correctly.\n";

        return false;
    }

    TestCase failedBeforeSkip(
        "skipping.failure_before_skip",
        "Failure Before Skip",
        [](TestContext &context)
        {
            context.addFailure(TestFailure(
                "Failure recorded before skip",
                std::source_location::current()));

            context.skip("Later condition was unavailable");
        });

    const TestResult failed = runner.run(failedBeforeSkip);

    if (failed.status() != TestStatus::Failed ||
        failed.failures().empty() ||
        failed.skipReason() != "Later condition was unavailable")
    {
        std::cerr
            << "Self-test error: a failure recorded before skip must remain Failed.\n";

        return false;
    }

    try
    {
        CurrentTestContext::get();

        std::cerr
            << "Self-test error: CurrentTestContext was not cleared after skip.\n";

        return false;
    }
    catch (const std::logic_error &)
    {
        // Expected: no active test context should remain.
    }

    return true;
}