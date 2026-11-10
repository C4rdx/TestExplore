#ifndef CURRENTTESTCONTEXT_HPP
#define CURRENTTESTCONTEXT_HPP

namespace testexplorer
{
    class TestContext;

    class CurrentTestContext
    {
    public:
        class Scope final
        {
        public:
            explicit Scope(TestContext &context) noexcept;
            ~Scope() noexcept;

            Scope(const Scope &) = delete;
            Scope &operator=(const Scope &) = delete;
            Scope(Scope &&) = delete;
            Scope &operator=(Scope &&) = delete;

        private:
            TestContext *m_previousContext;
        };

        static void set(TestContext &context);
        static void clear();

        static TestContext &get();

    private:
        static TestContext *exchange(TestContext *context) noexcept;
    };
}

#endif