#pragma once

#include <vector>
#include <memory>

namespace Engine1
{
    class TextFile
    {
        public:

        static std::shared_ptr< std::vector<char> > load( const std::string& path );
    };
}

