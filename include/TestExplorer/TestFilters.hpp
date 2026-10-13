#ifndef TESTFILTERS_HPP
#define TESTFILTERS_HPP

#include <algorithm>
#include <string>
#include <utility>

#include <TestExplorer/TestCase.hpp>
#include <TestExplorer/TestFilter.hpp>

namespace testexplorer::filters
{
    inline TestFilter inSuite(std::string suite)
    {
        return [suite = std::move(suite)](const TestCase &test)
        {
            return test.metadata().suite == suite;
        };
    }

    inline TestFilter hasTag(std::string tag)
    {
        return [tag = std::move(tag)](const TestCase &test)
        {
            const auto &tags = test.metadata().tags;

            return std::find(tags.begin(), tags.end(), tag) != tags.end();
        };
    }
}

#endif