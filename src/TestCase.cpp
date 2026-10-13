
#include <TestExplorer/TestCase.hpp>
#include <TestExplorer/TestContext.hpp>

#include <utility>

namespace testexplorer
{
    TestCase::TestCase(
        std::string id,
        std::string name,
        TestFunction function,
        TestMetadata metadata)
        : m_id(std::move(id)),
          m_name(std::move(name)),
          m_function(std::move(function)),
          m_metadata(std::move(metadata))
    {
    }

    void TestCase::execute(TestContext &context) const
    {
        m_function(context);
    }

    const std::string &TestCase::id() const
    {
        return m_id;
    }

    const std::string &TestCase::name() const
    {
        return m_name;
    }

    const TestMetadata &TestCase::metadata() const
    {
        return m_metadata;
    }
}
