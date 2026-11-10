#include <TestExplorer/CurrentTestContext.hpp>
#include <TestExplorer/TestContext.hpp>

#include <stdexcept>

namespace testexplorer
{
    namespace
    {
        thread_local TestContext *currentContext = nullptr;
    }

    TestContext *CurrentTestContext::exchange(TestContext *context) noexcept
    {
        TestContext *previousContext = currentContext;
        currentContext = context;
        return previousContext;
    }

    void CurrentTestContext::set(TestContext &context)
    {
        (void)exchange(&context);
    }

    void CurrentTestContext::clear()
    {
        (void)exchange(nullptr);
    }

    TestContext &CurrentTestContext::get()
    {
        if (currentContext == nullptr)
        {
            throw std::logic_error(
                "No active TestContext");
        }

        return *currentContext;
    }

    CurrentTestContext::Scope::Scope(TestContext &context) noexcept
        : m_previousContext(CurrentTestContext::exchange(&context))
    {
    }

    CurrentTestContext::Scope::~Scope() noexcept
    {
        (void)CurrentTestContext::exchange(m_previousContext);
    }
}