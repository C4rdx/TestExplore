#ifndef TESTCASE_HPP
#define TESTCASE_HPP

#include <functional>
#include <string>

#include <TestExplorer/TestMetadata.hpp>

namespace testexplorer
{
    class TestContext;

    class TestCase
    {
    public:
        using TestFunction = std::function<void(TestContext &)>;

        TestCase(
            std::string id,
            std::string name,
            TestFunction function,
            TestMetadata metadata = {});

        void execute(TestContext &context) const;

        const std::string &id() const;
        const std::string &name() const;
        const TestMetadata &metadata() const;

    private:
        std::string m_id;
        std::string m_name;
        TestFunction m_function;
        TestMetadata m_metadata;
    };
}

#endif
