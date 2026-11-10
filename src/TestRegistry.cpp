#include <TestExplorer/TestRegistry.hpp>

#include <stdexcept>
#include <utility>

namespace testexplorer
{
    void TestRegistry::registerTest(TestCase test)
    {
        if (find(test.id()) != nullptr)
        {
            throw std::invalid_argument(
                "A test with ID '" + test.id() + "' is already registered.");
        }

        m_tests.push_back(std::move(test));
    }

    const std::vector<TestCase> &TestRegistry::tests() const
    {
        return m_tests;
    }

    const TestCase *TestRegistry::find(const std::string &id) const
    {
        for (const TestCase &test : m_tests)
        {
            if (test.id() == id)
            {
                return &test;
            }
        }

        return nullptr;
    }
}