
#ifndef TESTMETADATA_HPP
#define TESTMETADATA_HPP

#include <string>
#include <vector>

namespace testexplorer
{
    struct TestMetadata
    {
        std::string suite;
        std::vector<std::string> tags;
    };
}

#endif
