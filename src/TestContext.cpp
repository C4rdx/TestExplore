#include <TestExplorer/TestContext.hpp>
#include <TestExplorer/TestSkipException.hpp>

#include <utility>

namespace testexplorer
{
    void TestContext::addFailure(TestFailure failure)
    {
        m_failures.push_back(std::move(failure));
    }

    const std::vector<TestFailure> &TestContext::failures() const
    {
        return m_failures;
    }

    [[noreturn]] void TestContext::skip(std::string reason) const
    {
        throw detail::TestSkipException(std::move(reason));
    }
}