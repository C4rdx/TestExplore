#include "test_runner.hpp"

#include <TestExplorer/TestCase.hpp>
#include <TestExplorer/TestResult.hpp>
#include <TestExplorer/TestFilters.hpp>
#include <TestExplorer/TestRegistry.hpp>

#include <string>
#include <vector>
#include <iostream>
#include <stdexcept>

using namespace testexplorer;

namespace
{
    bool verifyMathFilter(
        const std::vector<testexplorer::TestResult> &results)
    {
        if (results.size() != 2)
        {
            std::cerr
                << "Self-test error: expected 2 filtered results, got "
                << results.size()
                << '\n';

            return false;
        }

        const testexplorer::TestResult *addition = nullptr;
        const testexplorer::TestResult *subtraction = nullptr;

        for (const auto &result : results)
        {
            if (result.testId() == "math.addition")
            {
                addition = &result;
            }
            else if (result.testId() == "math.subtraction")
            {
                subtraction = &result;
            }
        }

        if (addition == nullptr ||
            subtraction == nullptr)
        {
            std::cerr
                << "Self-test error: math filter returned incorrect tests.\n";

            return false;
        }

        if (addition->status() != testexplorer::TestStatus::Passed ||
            subtraction->status() != testexplorer::TestStatus::Passed)
        {
            std::cerr
                << "Self-test error: filtered tests should pass.\n";

            return false;
        }

        return true;
    }

    bool verifyEmptyFilter(
        const std::vector<testexplorer::TestResult> &results)
    {
        if (!results.empty())
        {
            std::cerr
                << "Self-test error: expected no tests to match filter, got "
                << results.size()
                << '\n';

            return false;
        }

        return true;
    }

    bool verifyRunSingle(
        const testexplorer::TestResult &result)
    {
        if (result.testId() != "math.addition" ||
            result.testName() != "Math Addition" ||
            result.status() != testexplorer::TestStatus::Passed)
        {
            std::cerr
                << "Self-test error: single test execution returned an unexpected result.\n";

            return false;
        }

        return true;
    }

    bool verifyRunFailed(
        const std::vector<testexplorer::TestResult> &results)
    {
        if (results.size() != 2)
        {
            std::cerr
                << "Self-test error: expected 2 failed tests to be rerun, got "
                << results.size()
                << '\n';

            return false;
        }

        bool foundFailingAssertions = false;
        bool foundMultipleFailures = false;

        for (const auto &result : results)
        {
            if (result.testId() == "assertions.failing")
            {
                foundFailingAssertions = true;
            }
            else if (result.testId() == "assertions.multiple_failures")
            {
                foundMultipleFailures = true;
            }
        }

        if (!foundFailingAssertions ||
            !foundMultipleFailures)
        {
            std::cerr
                << "Self-test error: runFailed returned incorrect tests.\n";

            return false;
        }

        for (const auto &result : results)
        {
            if (result.status() != testexplorer::TestStatus::Failed)
            {
                std::cerr
                    << "Self-test error: rerun failed tests should still fail.\n";

                return false;
            }
        }

        return true;
    }

    bool verifyMetadataFilters(testexplorer::TestRunner &runner)
    {
        testexplorer::TestRegistry registry;

        registry.registerTest(testexplorer::TestCase(
            "database.connection",
            "Database Connection",
            [](testexplorer::TestContext &) {},
            {"Database", {"integration", "slow"}}));

        registry.registerTest(testexplorer::TestCase(
            "database.unit",
            "Database Unit Test",
            [](testexplorer::TestContext &) {},
            {"Database", {"unit"}}));

        registry.registerTest(testexplorer::TestCase(
            "math.addition",
            "Math Addition",
            [](testexplorer::TestContext &) {},
            {"Math", {"unit"}}));

        // Verify backward compatibility with the original 3-argument constructor.
        registry.registerTest(testexplorer::TestCase(
            "legacy.test",
            "Legacy Test",
            [](testexplorer::TestContext &) {}));

        // Filter by suite.
        const auto databaseResults = runner.runAll(
            registry,
            testexplorer::filters::inSuite("Database"));

        if (databaseResults.size() != 2)
        {
            std::cerr << "Self-test error: Database suite should contain 2 tests.\n";
            return false;
        }

        // Filter by tag.
        const auto integrationResults = runner.runAll(
            registry,
            testexplorer::filters::hasTag("integration"));

        if (integrationResults.size() != 1 ||
            integrationResults.front().testId() != "database.connection")
        {
            std::cerr << "Self-test error: integration tag filter failed.\n";
            return false;
        }

        // Combine suite and tag filters.
        const auto databaseUnitResults = runner.runAll(
            registry,
            [suiteFilter = testexplorer::filters::inSuite("Database"),
             tagFilter = testexplorer::filters::hasTag("unit")](const testexplorer::TestCase &test)
            {
                return suiteFilter(test) && tagFilter(test);
            });

        if (databaseUnitResults.size() != 1 ||
            databaseUnitResults.front().testId() != "database.unit")
        {
            std::cerr << "Self-test error: combined metadata filters failed.\n";
            return false;
        }

        // A nonexistent suite should match no tests.
        const auto missingSuiteResults = runner.runAll(
            registry,
            testexplorer::filters::inSuite("MissingSuite"));

        if (!missingSuiteResults.empty())
        {
            std::cerr << "Self-test error: nonexistent suite should match no tests.\n";
            return false;
        }

        // The original constructor must remain compatible.
        const auto legacyResult = runner.run(
            registry,
            "legacy.test");

        if (legacyResult.status() != testexplorer::TestStatus::Passed)
        {
            std::cerr << "Self-test error: legacy TestCase construction failed.\n";
            return false;
        }

        return true;
    }
}

bool runRunnerTests(
    testexplorer::TestRunner &runner,
    testexplorer::TestRegistry &registry)
{
    // Test suite and tag metadata filters
    if (!verifyMetadataFilters(runner))
    {
        return false;
    }

    // Run all tests
    const auto results =
        runner.runAll(registry);

    if (results.size() != registry.tests().size())
    {
        std::cerr
            << "Self-test error: runAll result count mismatch.\n";

        return false;
    }

    // Math filter
    const auto mathResults =
        runner.runAll(
            registry,
            [](const testexplorer::TestCase &test)
            {
                return test.id().starts_with("math.");
            });

    if (!verifyMathFilter(mathResults))
    {
        return false;
    }

    // Empty filter
    const auto emptyResults =
        runner.runAll(
            registry,
            [](const testexplorer::TestCase &test)
            {
                return test.id().starts_with("database.");
            });

    if (!verifyEmptyFilter(emptyResults))
    {
        return false;
    }

    // All filter
    const auto allResults =
        runner.runAll(
            registry,
            [](const testexplorer::TestCase &)
            {
                return true;
            });

    if (allResults.size() != registry.tests().size())
    {
        std::cerr
            << "Self-test error: all filter should select every test.\n";

        return false;
    }

    // Run single test
    const TestResult singleResult =
        runner.run(
            registry,
            "math.addition");

    if (!verifyRunSingle(singleResult))
    {
        return false;
    }

    // Invalid test ID
    try
    {
        runner.run(
            registry,
            "does.not.exist");

        std::cerr
            << "Self-test error: missing test ID should throw.\n";

        return false;
    }
    catch (const std::invalid_argument &)
    {
        // Expected.
    }

    // Rerun previously failed tests
    const auto failedResults =
        runner.runFailed(
            registry,
            results);

    if (!verifyRunFailed(failedResults))
    {
        return false;
    }

    // Run failed tests when there are no failures
    const auto noFailedResults =
        runner.runFailed(
            registry,
            mathResults);

    if (!noFailedResults.empty())
    {
        std::cerr
            << "Self-test error: expected no failed tests to rerun.\n";

        return false;
    }

    return true;
}