#pragma once

#include <vector>
#include <memory>

namespace Engine1
{
    class TextFile
    {
        public:

        //#TODO: delete constructors/destructors.

        static std::shared_ptr< std::vector<char> > load( const std::string& path );
        static void                                 save( const std::string& path, std::vector< char >& data );
    };
}

