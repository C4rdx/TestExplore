#ifndef TESTRUNNER_HPP
#define TESTRUNNER_HPP

#include <TestExplorer/TestFilter.hpp>
#include <TestExplorer/TestResult.hpp>
#include <TestExplorer/TestSkipException.hpp>
    
#include <vector>

namespace testexplorer
{
    class TestCase;
    class TestRegistry;
    class TestReporter;

    class TestRunner
    {
    public:
        explicit TestRunner(
            TestReporter *reporter = nullptr);

        TestResult run(
            const TestCase &test);

        TestResult run(
            const TestRegistry &registry,
            const std::string &testId);

        std::vector<TestResult> runAll(
            const TestRegistry &registry);

        std::vector<TestResult> runAll(
            const TestRegistry &registry,
            TestFilter filter);

        std::vector<TestResult> runFailed(
            const TestRegistry &registry,
            const std::vector<TestResult> &previousResults);

    private:
        TestReporter *m_reporter;
    };
}

#endif
