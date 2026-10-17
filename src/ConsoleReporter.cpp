#include <TestExplorer/ConsoleReporter.hpp>

#include <TestExplorer/TestCase.hpp>
#include <TestExplorer/TestResult.hpp>

#include <chrono>
#include <cstddef>
#include <iostream>

namespace testexplorer
{
    namespace
    {
        const char *statusToString(TestStatus status)
        {
            switch (status)
            {
            case TestStatus::Passed:
                return "PASSED";

            case TestStatus::Failed:
                return "FAILED";

            case TestStatus::Skipped:
                return "SKIPPED";

            case TestStatus::Error:
                return "ERROR";
            }

            return "UNKNOWN";
        }
    }

    ConsoleReporter::ConsoleReporter(std::ostream &output) : m_output(output) {}

    void ConsoleReporter::testStarted(const TestCase &test)
    {
        // Results are printed after test execution.
        (void)test;
    }

    void ConsoleReporter::testFinished(const TestResult &result)
    {
        m_output
            << result.testName()
            << " ["
            << result.testId()
            << "]: "
            << statusToString(result.status())
            << " | "
            << result.duration().count()
            << " ns\n";

        for (const TestFailure &failure : result.failures())
        {
            m_output
                << "  Message: "
                << failure.message()
                << '\n';

            m_output
                << "  Location: "
                << failure.location().file_name()
                << ':'
                << failure.location().line()
                << '\n';
        }

        if (result.status() == TestStatus::Error)
        {
            m_output
                << "  Error: "
                << result.errorMessage()
                << '\n';
        }
    }

    void ConsoleReporter::testRunFinished(
        const std::vector<TestResult> &results)
    {
        std::size_t passed = 0;
        std::size_t failed = 0;
        std::size_t errors = 0;
        std::size_t skipped = 0;

        TestResult::Duration totalDuration{};

        for (const TestResult &result : results)
        {
            totalDuration += result.duration();

            switch (result.status())
            {
            case TestStatus::Passed:
                ++passed;
                break;

            case TestStatus::Failed:
                ++failed;
                break;

            case TestStatus::Error:
                ++errors;
                break;

            case TestStatus::Skipped:
                ++skipped;
                break;
            }
        }

        m_output
            << "\n=== Test Run Summary ===\n"
            << "Total:    " << results.size() << '\n'
            << "Passed:   " << passed << '\n'
            << "Failed:   " << failed << '\n'
            << "Errors:   " << errors << '\n'
            << "Skipped:  " << skipped << '\n'
            << "Duration: " << totalDuration.count() << " ns\n";
    }
}