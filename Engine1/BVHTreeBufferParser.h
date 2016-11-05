#pragma once

#include <vector>
#include <memory>

namespace Engine1
{
    class BVHTreeBuffer;

    class BVHTreeBufferParser
    {
        public:
        static std::shared_ptr< BVHTreeBuffer > parseBVHTreeFile( std::vector<char>::const_iterator& dataIt, std::vector<char>::const_iterator& dataEndIt );

        static void writeBVHTreeFile( std::vector< char >& data, const BVHTreeBuffer& bvhTree );
        static int  getSizeOfBVHTreeFile( const BVHTreeBuffer& bvhTree );

        private:

        BVHTreeBufferParser();
        ~BVHTreeBufferParser();
    };
}

