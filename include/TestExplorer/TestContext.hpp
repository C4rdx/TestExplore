#ifndef TESTCONTEXT_HPP
#define TESTCONTEXT_HPP

#include <TestExplorer/TestFailure.hpp>

#include <string>
#include <vector>

namespace testexplorer
{

    class TestContext
    {
    public:
        void addFailure(TestFailure failure);

        const std::vector<TestFailure> &failures() const;

        [[noreturn]] void skip(std::string reason) const;

    private:
        std::vector<TestFailure> m_failures;
    };

}

#endif