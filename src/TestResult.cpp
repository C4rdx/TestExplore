#include <TestExplorer/TestResult.hpp>

#include <utility>

namespace testexplorer
{
    TestResult::TestResult(
        std::string testId,
        std::string testName,
        TestStatus status,
        Duration duration,
        std::vector<TestFailure> failures,
        std::string errorMessage,
        std::string skipReason)
        : m_testId(std::move(testId)),
          m_testName(std::move(testName)),
          m_status(status),
          m_duration(duration),
          m_failures(std::move(failures)),
          m_errorMessage(std::move(errorMessage)),
          m_skipReason(std::move(skipReason))
    {
    }

    const std::string &TestResult::testId() const
    {
        return m_testId;
    }

    const std::string &TestResult::testName() const
    {
        return m_testName;
    }

    TestStatus TestResult::status() const
    {
        return m_status;
    }

    TestResult::Duration TestResult::duration() const
    {
        return m_duration;
    }

    const std::vector<TestFailure> &TestResult::failures() const
    {
        return m_failures;
    }

    const std::string &TestResult::errorMessage() const
    {
        return m_errorMessage;
    }

    const std::string &TestResult::skipReason() const
    {
        return m_skipReason;
    }
}