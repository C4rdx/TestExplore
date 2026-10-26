#include <TestExplorer/TestRunner.hpp>
#include <TestExplorer/CurrentTestContext.hpp>
#include <TestExplorer/TestCase.hpp>
#include <TestExplorer/TestContext.hpp>
#include <TestExplorer/TestRegistry.hpp>
#include <TestExplorer/TestReporter.hpp>
#include <TestExplorer/TestSkipException.hpp>

#include <chrono>
#include <stdexcept>
#include <string>
#include <vector>

namespace testexplorer
{
    namespace
    {
        class TestContextGuard
        {
        public:
            ~TestContextGuard()
            {
                CurrentTestContext::clear();
            }
        };
    }

    TestRunner::TestRunner(TestReporter *reporter)
        : m_reporter(reporter)
    {
    }

    TestResult TestRunner::run(const TestCase &test)
    {
        if (m_reporter != nullptr)
        {
            m_reporter->testStarted(test);
        }

        TestResult result(
            test.id(),
            test.name(),
            TestStatus::Error,
            TestResult::Duration::zero(),
            {});

        {
            TestContext context;

            CurrentTestContext::set(context);
            TestContextGuard contextGuard;

            const auto start = std::chrono::steady_clock::now();

            TestStatus status = TestStatus::Passed;
            std::string errorMessage;
            std::string skipReason;

            try
            {
                test.execute(context);

                status = context.failures().empty()
                             ? TestStatus::Passed
                             : TestStatus::Failed;
            }
            catch (const detail::TestSkipException &exception)
            {
                skipReason = exception.reason();

                status = context.failures().empty()
                             ? TestStatus::Skipped
                             : TestStatus::Failed;
            }
            catch (const std::exception &exception)
            {
                status = TestStatus::Error;
                errorMessage = exception.what();
            }
            catch (...)
            {
                status = TestStatus::Error;
                errorMessage = "Unknown exception";
            }

            const auto end = std::chrono::steady_clock::now();

            const auto duration =
                std::chrono::duration_cast<TestResult::Duration>(
                    end - start);

            result = TestResult(
                test.id(),
                test.name(),
                status,
                duration,
                context.failures(),
                errorMessage,
                skipReason);
        }

        if (m_reporter != nullptr)
        {
            m_reporter->testFinished(result);
        }

        return result;
    }

    TestResult TestRunner::run(
        const TestRegistry &registry,
        const std::string &testId)
    {
        const TestCase *test = registry.find(testId);

        if (test == nullptr)
        {
            throw std::invalid_argument(
                "Test not found: " + testId);
        }

        return run(*test);
    }

    std::vector<TestResult> TestRunner::runAll(
        const TestRegistry &registry)
    {
        std::vector<TestResult> results;

        for (const TestCase &test : registry.tests())
        {
            results.push_back(run(test));
        }

        if (m_reporter != nullptr)
        {
            m_reporter->testRunFinished(results);
        }

        return results;
    }

    std::vector<TestResult> TestRunner::runAll(
        const TestRegistry &registry,
        TestFilter filter)
    {
        std::vector<TestResult> results;

        for (const TestCase &test : registry.tests())
        {
            if (!filter(test))
            {
                continue;
            }

            results.push_back(run(test));
        }

        if (m_reporter != nullptr)
        {
            m_reporter->testRunFinished(results);
        }

        return results;
    }

    std::vector<TestResult> TestRunner::runFailed(
        const TestRegistry &registry,
        const std::vector<TestResult> &previousResults)
    {
        std::vector<TestResult> results;

        for (const TestResult &previousResult : previousResults)
        {
            if (previousResult.status() != TestStatus::Failed)
            {
                continue;
            }

            const TestCase *test =
                registry.find(previousResult.testId());

            if (test == nullptr)
            {
                continue;
            }

            results.push_back(run(*test));
        }

        if (m_reporter != nullptr)
        {
            m_reporter->testRunFinished(results);
        }

        return results;
    }
}