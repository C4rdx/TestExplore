#include "test_stability.hpp"

#include <TestExplorer/TestCase.hpp>
#include <TestExplorer/Assertions.hpp>
#include <TestExplorer/TestResult.hpp>
#include <TestExplorer/TestReporter.hpp>
#include <TestExplorer/TestRegistry.hpp>

#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>

using namespace testexplorer;

namespace
{
    class RecordingReporter final : public testexplorer::TestReporter
    {
    public:
        int startedCount = 0;
        int finishedCount = 0;
        int runFinishedCount = 0;

        std::vector<std::string> startedTestIds;
        std::vector<std::string> finishedTestIds;
        std::vector<
            std::vector<testexplorer::TestResult>>
            completedRuns;

        void testStarted(
            const testexplorer::TestCase &test) override
        {
            ++startedCount;
            startedTestIds.push_back(test.id());
        }

        void testFinished(
            const testexplorer::TestResult &result) override
        {
            ++finishedCount;
            finishedTestIds.push_back(result.testId());
        }

        void testRunFinished(
            const std::vector<testexplorer::TestResult> &results) override
        {
            ++runFinishedCount;
            completedRuns.push_back(results);
        }
    };

    bool verifyEmptyRegistry(
        testexplorer::TestRunner &runner)
    {
        testexplorer::TestRegistry emptyRegistry;

        const auto results =
            runner.runAll(emptyRegistry);

        if (!results.empty())
        {
            std::cerr
                << "Self-test error: expected empty registry to produce no results.\n";

            return false;
        }

        return true;
    }

    bool verifyEmptyPreviousResults(
        testexplorer::TestRunner &runner,
        testexplorer::TestRegistry &registry)
    {
        const std::vector<testexplorer::TestResult> previousResults;

        const auto results =
            runner.runFailed(
                registry,
                previousResults);

        if (!results.empty())
        {
            std::cerr
                << "Self-test error: expected no failed tests from empty previous results.\n";

            return false;
        }

        return true;
    }

    bool verifyRunFailedIgnoresNonFailedResults(
        testexplorer::TestRunner &runner,
        testexplorer::TestRegistry &registry)
    {
        const std::vector<testexplorer::TestResult> previousResults =
            {
                testexplorer::TestResult(
                    "synthetic.passed",
                    "Synthetic Passed",
                    testexplorer::TestStatus::Passed,
                    testexplorer::TestResult::Duration::zero(),
                    {}),
                testexplorer::TestResult(
                    "synthetic.skipped",
                    "Synthetic Skipped",
                    testexplorer::TestStatus::Skipped,
                    testexplorer::TestResult::Duration::zero(),
                    {}),
                testexplorer::TestResult(
                    "synthetic.error",
                    "Synthetic Error",
                    testexplorer::TestStatus::Error,
                    testexplorer::TestResult::Duration::zero(),
                    {})};

        const auto results =
            runner.runFailed(
                registry,
                previousResults);

        if (!results.empty())
        {
            std::cerr
                << "Self-test error: runFailed should ignore non-failed results.\n";

            return false;
        }

        return true;
    }

    bool verifyRunFailedIgnoresMissingTests(
        testexplorer::TestRunner &runner,
        testexplorer::TestRegistry &registry)
    {
        const std::vector<testexplorer::TestResult> previousResults =
            {
                testexplorer::TestResult(
                    "does.not.exist",
                    "Missing Test",
                    testexplorer::TestStatus::Failed,
                    testexplorer::TestResult::Duration::zero(),
                    {})};

        const auto results =
            runner.runFailed(
                registry,
                previousResults);

        if (!results.empty())
        {
            std::cerr
                << "Self-test error: missing failed tests should be ignored.\n";

            return false;
        }

        return true;
    }

    bool verifyReporterLifecycle(
        testexplorer::TestRegistry &registry)
    {
        RecordingReporter reporter;
        testexplorer::TestRunner runner(&reporter);

        const testexplorer::TestCase *test =
            registry.find("assertions.passing");

        if (test == nullptr)
        {
            std::cerr
                << "Self-test error: lifecycle test case was not found.\n";

            return false;
        }

        const auto singleResult =
            runner.run(*test);

        if (singleResult.status() !=
            testexplorer::TestStatus::Passed)
        {
            std::cerr
                << "Self-test error: lifecycle single test should pass.\n";

            return false;
        }

        if (reporter.startedCount != 1 ||
            reporter.finishedCount != 1 ||
            reporter.runFinishedCount != 0)
        {
            std::cerr
                << "Self-test error: invalid single-test reporter lifecycle.\n";

            return false;
        }

        const auto results =
            runner.runAll(registry);

        const auto expectedEventCount =
            registry.tests().size() + 1;

        if (reporter.startedCount !=
                static_cast<int>(expectedEventCount) ||
            reporter.finishedCount !=
                static_cast<int>(expectedEventCount) ||
            reporter.runFinishedCount != 1)
        {
            std::cerr
                << "Self-test error: invalid runAll reporter lifecycle.\n";

            return false;
        }

        if (reporter.completedRuns.size() != 1 ||
            reporter.completedRuns.front().size() !=
                registry.tests().size())
        {
            std::cerr
                << "Self-test error: invalid runFinished result set.\n";

            return false;
        }

        if (results.size() != registry.tests().size())
        {
            std::cerr
                << "Self-test error: runAll result count mismatch.\n";

            return false;
        }

        return true;
    }

    bool verifyTestResultConsistency(
        testexplorer::TestRegistry &registry,
        testexplorer::TestRegistry &exceptionRegistry)
    {
        testexplorer::TestRunner runner;

        const testexplorer::TestCase *passedTest =
            registry.find("assertions.passing");

        const testexplorer::TestCase *failedTest =
            registry.find("assertions.failing");

        const testexplorer::TestCase *errorTest =
            exceptionRegistry.find(
                "exceptions.runtime_error");

        if (passedTest == nullptr ||
            failedTest == nullptr ||
            errorTest == nullptr)
        {
            std::cerr
                << "Self-test error: required TestResult consistency test case was not found.\n";

            return false;
        }

        const auto passed =
            runner.run(*passedTest);

        if (passed.status() !=
                testexplorer::TestStatus::Passed ||
            !passed.failures().empty() ||
            !passed.errorMessage().empty())
        {
            std::cerr
                << "Self-test error: invalid Passed TestResult state.\n";

            return false;
        }

        const auto failed =
            runner.run(*failedTest);

        if (failed.status() !=
                testexplorer::TestStatus::Failed ||
            failed.failures().empty() ||
            !failed.errorMessage().empty())
        {
            std::cerr
                << "Self-test error: invalid Failed TestResult state.\n";

            return false;
        }

        const auto error =
            runner.run(*errorTest);

        if (error.status() !=
                testexplorer::TestStatus::Error ||
            error.errorMessage().empty())
        {
            std::cerr
                << "Self-test error: invalid Error TestResult state.\n";

            return false;
        }

        return true;
    }

    bool verifyNestedExecutionRestoresContext()
    {
        testexplorer::TestRunner nestedRunner;

        bool innerPassed = false;
        bool innerErrorWasCaptured = false;

        testexplorer::TestCase passingInnerTest(
            "stability.nested.passing",
            "Nested Passing Test",
            [](testexplorer::TestContext &)
            {
                EXPECT_TRUE(true);
            });

        testexplorer::TestCase throwingInnerTest(
            "stability.nested.throwing",
            "Nested Throwing Test",
            [](testexplorer::TestContext &)
            {
                throw std::runtime_error("nested execution failure");
            });

        testexplorer::TestCase outerTest(
            "stability.nested.outer",
            "Outer Test After Nested Execution",
            [&nestedRunner,
             &passingInnerTest,
             &throwingInnerTest,
             &innerPassed,
             &innerErrorWasCaptured](testexplorer::TestContext &)
            {
                EXPECT_TRUE(true);

                const auto passingResult =
                    nestedRunner.run(passingInnerTest);

                innerPassed =
                    passingResult.status() == testexplorer::TestStatus::Passed;

                const auto throwingResult =
                    nestedRunner.run(throwingInnerTest);

                innerErrorWasCaptured =
                    throwingResult.status() == testexplorer::TestStatus::Error &&
                    throwingResult.errorMessage() == "nested execution failure";

                // The outer context must be restored after both nested runs.
                EXPECT_TRUE(true);
            });

        const auto outerResult = nestedRunner.run(outerTest);

        if (!innerPassed ||
            !innerErrorWasCaptured ||
            outerResult.status() != testexplorer::TestStatus::Passed)
        {
            std::cerr
                << "Self-test error: nested test execution did not restore the outer context.\n";

            return false;
        }

        return true;
    }

    bool verifyRepeatedExecutionIsolation()
    {
        testexplorer::TestRunner runner;

        testexplorer::TestCase test(
            "stability.repeated",
            "Repeated Execution",
            [](testexplorer::TestContext &context)
            {
                EXPECT_TRUE(context.failures().empty());
            });

        const auto first =
            runner.run(test);

        const auto second =
            runner.run(test);

        if (first.status() !=
                testexplorer::TestStatus::Passed ||
            second.status() !=
                testexplorer::TestStatus::Passed)
        {
            std::cerr
                << "Self-test error: repeated execution should remain isolated.\n";

            return false;
        }

        if (!first.failures().empty() ||
            !second.failures().empty())
        {
            std::cerr
                << "Self-test error: repeated execution leaked failures.\n";

            return false;
        }

        return true;
    }
}

bool runStabilityTests(
    testexplorer::TestRunner &runner,
    testexplorer::TestRegistry &registry,
    testexplorer::TestRegistry &exceptionRegistry)
{
    if (!verifyEmptyRegistry(runner))
    {
        return false;
    }

    if (!verifyEmptyPreviousResults(
            runner,
            registry))
    {
        return false;
    }

    if (!verifyRunFailedIgnoresNonFailedResults(
            runner,
            registry))
    {
        return false;
    }

    if (!verifyRunFailedIgnoresMissingTests(
            runner,
            registry))
    {
        return false;
    }

    if (!verifyReporterLifecycle(registry))
    {
        return false;
    }

    if (!verifyTestResultConsistency(
            registry,
            exceptionRegistry))
    {
        return false;
    }

    if (!verifyNestedExecutionRestoresContext())
    {
        return false;
    }

    if (!verifyRepeatedExecutionIsolation())
    {
        return false;
    }

    return true;
}