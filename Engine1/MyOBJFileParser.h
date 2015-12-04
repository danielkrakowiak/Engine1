#pragma once

#include <sstream>
#include <vector>
#include <tuple>
#include <memory>

#include "float2.h"
#include "float3.h"
#include "uint2.h"
#include "uint3.h"



namespace Engine1
{
    class BlockMesh;

    class MyOBJFileParser
    {
        private:
        MyOBJFileParser();
        ~MyOBJFileParser();

        public:
        static std::shared_ptr<BlockMesh> parseBlockMeshFile( const std::vector<char>& file, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs );

        private:
        static const int reusedVerticesMaxChecksCount = 50;

        static float2 parseFloat2( std::vector<char>::const_iterator& it );
        static float3 parseFloat3( std::vector<char>::const_iterator& it );

        static uint3 parseVertexTexcoordNormalIndices( std::vector<char>::const_iterator& it );
        static uint2 parseVertexTexcoordIndices( std::vector<char>::const_iterator& it );
        static uint2 parseVertexNormalIndices( std::vector<char>::const_iterator& it );
        static unsigned int parseVertexIndex( std::vector<char>::const_iterator& it );

        static std::tuple<bool, unsigned int> lookForReusedVertex( uint3 vertexTexcoordNormalIndices, std::vector<uint3>::const_iterator vertexTexcoordNormalIndicesArrayEnd, std::vector<uint3>::const_iterator vertexTexcoordNormalIndicesArrayBegin );
        static std::tuple<bool, unsigned int> lookForReusedVertex( uint2 vertexTexcoordOrNormalIndices, std::vector<uint2>::const_iterator vertexTexcoordOrNormalIndicesArrayEnd, std::vector<uint2>::const_iterator vertexTexcoordOrNormalIndicesArrayBegin );
        static std::tuple<bool, unsigned int> lookForReusedVertex( unsigned int vertexIndices, std::vector<unsigned int>::const_iterator vertexIndicesArrayEnd, std::vector<unsigned int>::const_iterator vertexIndicesArrayBegin );

    };
}

