# TestExplorer

A lightweight, reusable, dependency-free C++20 testing framework built around a clean and extensible test execution architecture.

> **Status:** Early Development — The API and architecture may change before the first stable release.

## Overview

**TestExplorer** is an experimental C++ testing framework developed with a focus on:

- Clean architecture and separation of responsibilities
- Reusability and extensibility
- Explicit test execution and selection
- Assertion failure collection and useful diagnostics
- Exception-safe test execution
- Flexible reporting
- Minimal dependencies

The project is developed incrementally. The core execution model is established first, while higher-level capabilities are introduced as separate, testable milestones.

## Current Features

- Core test model: `TestCase`, `TestContext`, `TestFailure`, and `TestResult`
- Explicit test registration and lookup by ID
- Individual test execution and run-all execution
- Assertion functions and user-facing assertion macros
- Assertion failure collection with expected and actual values where applicable
- C++20 `std::source_location` support
- Per-test execution timing
- Exception handling and error results
- Automatic cleanup of the active test context
- Test filtering
- Test suites and tags
- Single-test execution by ID
- Rerunning previously failed tests
- Reporter abstraction and console reporting
- Test run summaries with per-status counts and aggregate duration
- Framework self-tests

---

## Requirements

TestExplorer requires:

- **C++20**
- **CMake 3.20 or newer**
- A C++20-compatible compiler

The current development environment includes:

- Windows 11
- MSYS2 UCRT64
- GCC 16.2.0
- CMake 4.1.2
- MinGW Makefiles

The framework is designed to remain dependency-free and rely primarily on the C++ standard library.

## Building

Clone the repository:

```bash
git clone https://github.com/C4rdx/TestExplore.git
cd TestExplore
```

Configure the project using the MinGW Makefiles generator:

```bash
cmake -S . -B build -G "MinGW Makefiles"
```

Build all configured targets:

```bash
cmake --build build --parallel
```

Run the framework self-tests from Windows PowerShell:

```powershell
.\build\TestExplorerSelfTests.exe
```

For clean builds, troubleshooting, and additional development commands, see [`BUILD.md`](BUILD.md).

---

## Assertions

TestExplorer provides a user-facing assertion API through macros:

```cpp
EXPECT_TRUE(condition);
EXPECT_FALSE(condition);
EXPECT_EQ(actual, expected);
EXPECT_NE(actual, expected);
```

For example:

```cpp
TestCase additionTest(
    "math.addition",
    "Addition Test",
    [](TestContext&)
    {
        EXPECT_EQ(2 + 2, 4);
        EXPECT_TRUE(10 > 5);
    }
);
```

The assertion macros provide a concise test-writing interface, while the underlying assertion functions perform the checks and record failures.

Assertion failures can include:

- A descriptive failure message
- Expected values, where applicable
- Actual values, where applicable
- The source location of the assertion

Example failure output:

```text
Expected values to be equal
Expected: world
Actual:   hello
```

The reported source location points to the assertion call inside the test, helping developers locate failures.

---

## Basic Usage

A typical TestExplorer workflow consists of:

1. Define a test using `TestCase`.
2. Register the test with `TestRegistry`.
3. Execute tests using `TestRunner`.
4. Inspect the resulting `TestResult` objects.

### 1. Define a Test

A test is represented by a `TestCase`, which contains a test ID, a display name, and a test function.

```cpp
TestCase additionTest(
    "math.addition",
    "Addition Test",
    [](TestContext&)
    {
        EXPECT_EQ(2 + 2, 4);
        EXPECT_TRUE(10 > 5);
    }
);
```

The test body receives a `TestContext`. The public assertion macros access the active context through TestExplorer's current-context mechanism.

### 2. Register the Test

Tests are explicitly registered in a `TestRegistry`:

```cpp
TestRegistry registry;
registry.registerTest(additionTest);
```

Multiple tests can be registered in the same registry:

```cpp
registry.registerTest(additionTest);
registry.registerTest(subtractionTest);
registry.registerTest(multiplicationTest);
```

### 3. Run the Tests

A `TestRunner` executes tests and produces a result for each execution:

```cpp
TestRunner runner;
const auto results = runner.runAll(registry);
```

### Complete Example

```cpp
#include <TestExplorer/Assertions.hpp>
#include <TestExplorer/TestCase.hpp>
#include <TestExplorer/TestRegistry.hpp>
#include <TestExplorer/TestRunner.hpp>

using namespace testexplorer;

int main()
{
    TestCase additionTest(
        "math.addition",
        "Addition Test",
        [](TestContext&)
        {
            EXPECT_EQ(2 + 2, 4);
            EXPECT_TRUE(10 > 5);
        }
    );

    TestRegistry registry;
    registry.registerTest(additionTest);

    TestRunner runner;
    const auto results = runner.runAll(registry);

    return 0;
}
```

---

## Test Results

Every test execution produces a `TestResult` containing information such as:

- Test ID
- Test name
- Execution status
- Execution duration
- Assertion failures
- Error information when execution throws an exception

The framework currently defines four statuses:

```cpp
enum class TestStatus
{
    Passed,
    Failed,
    Skipped,
    Error
};
```

### `Passed`

The test completed without assertion failures or execution errors.

### `Failed`

The test executed, but one or more assertions failed.

### `Skipped`

The test was intentionally not executed. Automatic skip-selection mechanisms may be expanded in future milestones.

### `Error`

An exception interrupted normal test execution. The runner converts the exception into an error result rather than allowing it to terminate the entire test run.

For example:

```cpp
TestCase connectionTest(
    "database.connection",
    "Database Connection",
    [](TestContext&)
    {
        throw std::runtime_error("Connection failed");
    }
);
```

For standard exceptions, the error message is obtained from `what()` and exposed through the result's `errorMessage()` accessor. For unknown exception types, the current fallback message is:

```text
Unknown exception
```

Assertion failures and execution errors are represented separately: assertion failures belong to the test context and result, while an execution exception produces the `Error` status.

Example of inspecting an error result:

```cpp
if (result.status() == TestStatus::Error)
{
    std::cout << result.errorMessage() << '\n';
}
```

---

## Test Filtering and Selection

TestExplorer supports selecting tests before execution through a filtering API.

A filter is represented by:

```cpp
using TestFilter = std::function<bool(const TestCase&)>;
```

The filter receives a test and returns `true` to execute it or `false` to exclude it.

For example, to execute only tests whose IDs begin with `math.`:

```cpp
const auto results = runner.runAll(
    registry,
    [](const TestCase& test)
    {
        return test.id().starts_with("math.");
    }
);
```

Filtering is separate from execution, allowing selection rules to evolve without duplicating the test execution logic.

### Single-Test Execution

A registered test can be executed directly by its ID:

```cpp
const TestResult result = runner.run(registry, "math.addition");
```

The registry-aware overload resolves the requested test through `TestRegistry` and delegates execution to the same underlying path used by other runner operations. If the requested test ID does not exist, the overload throws `std::invalid_argument`.

### Rerunning Failed Tests

TestExplorer can rerun tests that previously finished with `TestStatus::Failed`.

```cpp
const auto firstRun = runner.runAll(registry);
const auto failedResults = runner.runFailed(registry, firstRun);
```

The API is:

```cpp
std::vector<TestResult> runFailed(
    const TestRegistry& registry,
    const std::vector<TestResult>& previousResults
);
```

Only previously failed results are selected for rerun. Results with `Passed`, `Skipped`, or `Error` status are ignored. If a previously failed test is no longer registered, it is skipped.

The runner does not retain previous results internally; the caller owns and supplies them. This keeps test definitions, execution state, and previous results separate.

---

## Test Suites and Tags

TestExplorer supports organizing tests through metadata. A test can have a suite name and a collection of tags. Metadata is supplied as an optional fourth argument to `TestCase`, so existing three-argument test definitions remain valid.

### Defining Metadata

```cpp
#include <TestExplorer/Assertions.hpp>
#include <TestExplorer/TestCase.hpp>
#include <TestExplorer/TestMetadata.hpp>

using namespace testexplorer;

TestCase additionTest(
    "math.addition",
    "Addition Test",
    [](TestContext&)
    {
        EXPECT_EQ(2 + 2, 4);
    },
    TestMetadata{
        "Math",
        {"unit", "fast"}
    }
);
```

In this example, `Math` is the suite name and `unit` and `fast` are tags. Suites group related tests, while tags classify tests by properties or intended use.

### Filtering by Suite

The `TestFilters.hpp` header provides a helper for selecting a suite:

```cpp
#include <TestExplorer/TestFilters.hpp>

const auto results = runner.runAll(
    registry,
    filters::inSuite("Math")
);
```

### Filtering by Tag

To select tests containing a particular tag:

```cpp
const auto results = runner.runAll(
    registry,
    filters::hasTag("unit")
);
```

These helpers return ordinary `TestFilter` objects and work with the runner's existing filtering mechanism.

---

## Reporting

TestExplorer separates test execution from result presentation through the `TestReporter` abstraction. A reporter receives lifecycle events such as:

```cpp
testStarted(...);
testFinished(...);
testRunFinished(...);
```

This allows reporting implementations to observe individual tests and the completion of an entire test run without coupling output formatting to the execution core.

### Console Reporter

The current implementation includes `ConsoleReporter`:

```cpp
#include <iostream>
#include <TestExplorer/ConsoleReporter.hpp>
#include <TestExplorer/TestRunner.hpp>

ConsoleReporter reporter(std::cout);
TestRunner runner(&reporter);

const auto results = runner.runAll(registry);
```

The reporter displays test names, IDs, statuses, execution durations, failure details, and a final run summary.

Example output:

```text
Math Addition [math.addition]: PASSED | 400 ns
Math Subtraction [math.subtraction]: PASSED | 400 ns
Database Connection [database.connection]: ERROR | 1200 ns
  Error: Connection failed

=== Test Run Summary ===
Total:    3
Passed:   2
Failed:   0
Errors:   1
Skipped:  0
Duration: 2000 ns
```

The values above are illustrative. The summary includes the total number of results, per-status counts, and the aggregate duration of the reported test executions.

The reporter is injected into `TestRunner` as a non-owning dependency. The caller must ensure that the reporter remains alive while the runner uses it. Future reporters can be introduced without requiring the execution core to depend on a specific output format.

---

## Exception Handling and Context Cleanup

Exception handling is part of the execution core. A test throwing an exception should produce an error result rather than terminate the entire test run; execution can then continue with subsequent tests.

Standard exceptions are handled through:

```cpp
catch (const std::exception& exception)
```

The error message is obtained from `exception.what()`. Unknown exception types are handled through:

```cpp
catch (...)
```

The current fallback message is `Unknown exception`.

The active `TestContext` is temporary execution state. TestExplorer clears the current context after execution, including exception paths, so stale context state does not leak into subsequent tests. The cleanup mechanism uses RAII, tying cleanup to object lifetime rather than relying on every execution path to perform it manually.

---

## A More Realistic Example

Tests can contain multiple operations and assertions. TestExplorer does not impose a limit on the complexity of the test logic itself.

```cpp
#include <vector>
#include <TestExplorer/Assertions.hpp>
#include <TestExplorer/TestCase.hpp>

using namespace testexplorer;

TestCase vectorOperations(
    "vector.operations",
    "Vector Operations",
    [](TestContext&)
    {
        std::vector<int> values = {1, 2, 3, 4, 5};

        EXPECT_EQ(values.size(), 5);
        EXPECT_EQ(values.front(), 1);
        EXPECT_EQ(values.back(), 5);

        values.push_back(6);

        EXPECT_EQ(values.size(), 6);
        EXPECT_TRUE(values[5] == 6);
    }
);
```

The framework is responsible for executing the test, managing its context, collecting assertion failures, measuring execution time, handling execution exceptions, producing the corresponding `TestResult`, and reporting results when a reporter is configured. The logic being tested remains within the test itself.

---

## Architecture

TestExplorer is built around components with clearly separated responsibilities.

```text
                         Test Application
                                |
                                v
                            TestRunner
                                |
              +-----------------+-----------------+
              |                 |                 |
              v                 v                 v
         TestRegistry       TestContext        Reporter
              |                 |                 |
              v                 v                 v
           TestCase          Assertions       TestResult
                                                   |
                                                   v
                                              TestFailure
```

The intended dependency direction is:

```text
Consumer Tests / CLI / Examples
             |
             v
       TestExplorer Library
```

The execution core does not depend on a specific reporting implementation.

### Core Execution Flow

```text
TestRegistry
     |
     | stores registered tests
     v
  TestCase
     |
     | selected by ID or filter
     v
  TestRunner
     |
     +-- creates TestContext
     +-- activates CurrentTestContext
     +-- executes the test
     +-- handles exceptions
     +-- clears CurrentTestContext
     +-- measures execution duration
     +-- determines execution status
     +-- produces TestResult
              |
              v
           Reporter
```

The architecture aims to keep registration, selection, execution, assertion handling, result representation, and reporting independently maintainable.

---

## Core Components

### `TestCase`

`TestCase` represents a test definition. It contains a test ID, a display name, a test function, and optional metadata. A `TestCase` does not store its execution result; the same definition can be executed multiple times, producing independent results.

### `TestRegistry`

`TestRegistry` stores and organizes registered tests. Its current responsibilities include test registration, access to registered tests, and lookup by test ID. The registry does not execute tests.

### `TestRunner`

`TestRunner` executes tests and produces `TestResult` objects. Its responsibilities include:

- Individual test execution
- Single-test execution by ID
- Run-all execution
- Filtered execution
- Failed-test reruns
- Execution timing
- Assertion failure collection
- Exception handling and status determination
- Active context management
- Reporter notifications

The runner does not perform assertion comparisons, own the registry, or implement a specific output format.

### `TestContext`

`TestContext` represents the execution context of one test. It stores assertion failures generated during execution. A fresh context is created for each test execution.

### `CurrentTestContext`

`CurrentTestContext` provides temporary access to the context associated with the currently executing test. This mechanism allows assertions to use a straightforward interface:

```cpp
EXPECT_EQ(actual, expected);
```

instead of requiring the context to be passed explicitly to every assertion. The current context uses `thread_local` state and is cleared after execution, including exception paths.

### `TestFailure`

`TestFailure` represents a single assertion failure. It can contain a failure message, expected and actual values where applicable, and `std::source_location`. Execution exceptions are represented by the `Error` result status rather than as assertion failures.

### `TestResult`

`TestResult` represents one test execution. It contains the test identity, status, duration, assertion failures, and error information where applicable. Keeping `TestResult` separate from `TestCase` allows each execution to produce an independent result.

### `TestReporter`

`TestReporter` defines the interface for separating execution from result presentation. The current implementation includes `ConsoleReporter`. Future implementations may support JSON, XML, IDE integrations, or other structured output formats.

---

## Project Structure

The following is a high-level overview of the repository. The exact file list may evolve as development continues.

```text
TestExplore/
├── CMakeLists.txt
├── README.md
├── BUILD.md
├── .gitignore
│
├── apps/
│   └── testexplorer/
│       └── main.cpp
│
├── examples/
│   └── basic/
│       └── main.cpp
│
├── include/
│   └── TestExplorer/
│       ├── Assertions.hpp
│       ├── ConsoleReporter.hpp
│       ├── CurrentTestContext.hpp
│       ├── TestCase.hpp
│       ├── TestContext.hpp
│       ├── TestExplorer.hpp
│       ├── TestFailure.hpp
│       ├── TestFilter.hpp
│       ├── TestFilters.hpp
│       ├── TestMetadata.hpp
│       ├── TestRegistry.hpp
│       ├── TestReporter.hpp
│       ├── TestResult.hpp
│       └── TestRunner.hpp
│
├── src/
│   ├── ConsoleReporter.cpp
│   ├── CurrentTestContext.cpp
│   ├── TestCase.cpp
│   ├── TestContext.cpp
│   ├── TestExplorer.cpp
│   ├── TestFailure.cpp
│   ├── TestRegistry.cpp
│   ├── TestResult.cpp
│   └── TestRunner.cpp
│
└── tests/
    ├── main.cpp
    ├── test_assertions.cpp
    ├── test_assertions.hpp
    ├── test_exceptions.cpp
    ├── test_exceptions.hpp
    ├── test_registration.cpp
    ├── test_registration.hpp
    ├── test_runner.cpp
    ├── test_runner.hpp
    ├── test_stability.cpp
    └── test_stability.hpp
```

### Current Executables

| Target | Purpose | Status |
|---|---|---|
| `TestExplorerSelfTests` | Tests the framework itself | Active |
| `TestExplorerExample` | Demonstrates framework usage | Under development |
| `TestExplorerCLI` | Command-line test runner foundation | Under development |

The framework's own tests are maintained separately from consumer-facing examples and application scaffolding.

---

## Development Roadmap

The project is developed incrementally. The checklist below reflects the current implementation status.

### Core

- [x] Test case model
- [x] Test context
- [x] Failure representation
- [x] Test result model
- [x] Test registry
- [x] Test runner
- [x] Basic assertions
- [x] Single-test execution
- [x] Run-all execution
- [x] Execution timing
- [x] Current test context
- [x] Exception handling

### Assertions

- [x] Descriptive failure messages
- [x] Actual value reporting
- [x] Expected value reporting
- [x] Assertion source-location reporting
- [x] User-facing assertion macros

### Reporting

- [x] Reporter abstraction
- [x] Console reporter
- [x] Test run summaries
- [x] Per-status result counts
- [x] Aggregate execution duration
- [ ] JSON reporter
- [ ] XML reporter
- [ ] Additional structured output formats

### Test Organization

- [x] Test filtering
- [x] Single-test execution by ID
- [x] Failed-test reruns
- [x] Test suites
- [x] Tags and categories
- [ ] Test hierarchy

### Test Lifecycle

- [ ] Fixtures
- [ ] Setup and teardown
- [ ] Parameterized tests
- [ ] Test data support

### Integration

- [ ] Exit-code behavior
- [ ] CI/CD integration
- [ ] CLI test discovery and execution
- [ ] IDE integration

The roadmap is intentionally incremental. New features should be introduced when the underlying architecture can support them without unnecessary complexity.

---

## Design Goals

### Separation of Responsibilities

Test registration, execution, assertions, results, filtering, and reporting should remain separate responsibilities.

### Reusability

The core framework should be usable as a library rather than being tied to a particular application or IDE.

### Minimal Dependencies

The framework should rely primarily on the C++ standard library.

### Extensibility

New features, including reporters, filters, fixtures, and parameterized tests, should be possible without rewriting the core execution model.

### Clear Execution Model

A test definition should remain independent of its execution result.

### Explicit Test Selection

Test selection should remain separate from execution so that suites, tags, and future command-line filters can build on the same model.

### Reliable Execution

An exception inside one test should produce an error result rather than terminate the entire test run.

### Incremental Development

Features should be introduced in small, testable milestones rather than attempting to implement an entire testing ecosystem at once.

---

## Current Non-Goals

The following are intentionally outside the current scope:

- Graphical test explorer
- Reflection system
- Plugin architecture
- Distributed test execution
- Dependency injection
- Complex matcher framework
- Premature parallel execution

These may be reconsidered if actual project requirements justify them.

---

## Project Status

TestExplorer is an experimental open-source project under active development. The API is **not stable** and may change significantly before the first release.

The current implementation has established and validated:

- Core test execution
- Assertion handling and failure collection
- Test filtering, suites, and tags
- Single-test execution and failed-test reruns
- Reporter abstraction and console reporting
- Test run summaries
- Exception-safe execution and error results
- Framework self-tests

The next development milestones will continue expanding test lifecycle, integration, and organization capabilities while keeping the core architecture small and maintainable.

---

## License

A project license will be added before the first public release.
