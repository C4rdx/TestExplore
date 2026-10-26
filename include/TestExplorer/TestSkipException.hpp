#ifndef TESTSKIPEXCEPTION_HPP
#define TESTSKIPEXCEPTION_HPP

#include <string>
#include <utility>

namespace testexplorer::detail
{
    class TestSkipException
    {
    public:
        explicit TestSkipException(std::string reason)
            : m_reason(std::move(reason))
        {
        }

        [[nodiscard]] const std::string &reason() const noexcept
        {
            return m_reason;
        }

    private:
        std::string m_reason;
    };
}

#endif