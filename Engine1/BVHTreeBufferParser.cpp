#include "BVHTreeBufferParser.h"

#include "BVHTreeBuffer.h"

using namespace Engine1;

BVHTreeBufferParser::BVHTreeBufferParser()
{}

BVHTreeBufferParser::~BVHTreeBufferParser()
{}

std::shared_ptr< BVHTreeBuffer > BVHTreeBufferParser::parseBVHTreeFile( std::vector<char>::const_iterator& dataIt, std::vector<char>::const_iterator& dataEndIt )
{
    std::shared_ptr< BVHTreeBuffer > bvhTree = std::make_shared< BVHTreeBuffer >();

    const int dataSize = (int)( dataEndIt - dataIt ) / sizeof( char );

    std::vector<char>::const_iterator dataCurrIt = dataIt;

    // Read nodes count.
    int nodesCount = 0;
    std::memcpy( &nodesCount, &( *dataCurrIt ), sizeof( int ) );
    dataCurrIt += sizeof( int );

    // Read nodes.
    bvhTree->m_bvhNodes.resize( nodesCount );
    const int nodesDataSize = nodesCount * sizeof( BVHTreeBuffer::Node );
    std::memcpy( bvhTree->m_bvhNodes.data(), &( *dataCurrIt ), nodesDataSize );
    dataCurrIt += nodesDataSize;

    // Read nodes' extents.
    bvhTree->m_bvhNodesExtents.resize( nodesCount );
    const int nodesExtentsDataSize = nodesCount * sizeof( BVHTreeBuffer::NodeExtents );
    std::memcpy( bvhTree->m_bvhNodesExtents.data(), &( *dataCurrIt ), nodesExtentsDataSize );
    dataCurrIt += nodesExtentsDataSize;

    // Read triangle count.
    int triangleCount = 0;
    std::memcpy( &triangleCount, &( *dataCurrIt ), sizeof( int ) );
    dataCurrIt += sizeof( int );

    // Read triangles.
    bvhTree->m_triangles.resize( triangleCount );
    if ( triangleCount > 0 )
    {
        const int trianglesDataSize = triangleCount * sizeof( unsigned int );
        std::memcpy( bvhTree->m_triangles.data(), &( *dataCurrIt ), trianglesDataSize );
        dataCurrIt += trianglesDataSize;
    }

    return bvhTree;
}

void BVHTreeBufferParser::writeBVHTreeFile( std::vector< char >& data, const BVHTreeBuffer& bvhTree )
{
    const int totalSize = getSizeOfBVHTreeFile( bvhTree );

    const int prevDataSize = (int)data.size();
    data.resize( prevDataSize + totalSize );

    std::vector< char >::iterator dataIt = data.begin() + prevDataSize;

    // Write nodes count.
    const int nodesCount = (int)bvhTree.getNodes().size();
    std::memcpy( &( *dataIt ), &nodesCount, sizeof( int ) );
    dataIt += sizeof( int );

    // Write nodes.
    const int nodesDataSize = (int)bvhTree.getNodes().size() * sizeof( BVHTreeBuffer::Node );
    std::memcpy( &( *dataIt ), bvhTree.getNodes().data(), nodesDataSize );
    dataIt += nodesDataSize;

    // Write nodes' extents.
    const int nodesExtentsDataSize = (int)bvhTree.getNodesExtents().size() * sizeof( BVHTreeBuffer::NodeExtents );
    std::memcpy( &( *dataIt ), bvhTree.getNodesExtents().data(), nodesExtentsDataSize );
    dataIt += nodesExtentsDataSize;

    // Write triangle count.
    const int trianglesCount = (int)bvhTree.getTriangles().size();
    std::memcpy( &( *dataIt ), &trianglesCount, sizeof( int ) );
    dataIt += sizeof( int );

    // Write triangles.
    if ( !bvhTree.getTriangles().empty() )
    {
        const int trianglesDataSize = (int)bvhTree.getTriangles().size() * sizeof( unsigned int );
        std::memcpy( &( *dataIt ), bvhTree.getTriangles().data(), trianglesDataSize );
        dataIt += trianglesDataSize;
    }
}

int BVHTreeBufferParser::getSizeOfBVHTreeFile( const BVHTreeBuffer& bvhTree )
{
    const int nodesDataSize        = (int)bvhTree.getNodes().size() * sizeof( BVHTreeBuffer::Node );
    const int nodesExtentsDataSize = (int)bvhTree.getNodesExtents().size() * sizeof( BVHTreeBuffer::NodeExtents );
    const int trianglesDataSize    = (int)bvhTree.getTriangles().size() * sizeof( unsigned int );

    int totalSize = 0;
    totalSize += 2 * sizeof( int ); // Nodes count, triangles count.
    totalSize += nodesDataSize;
    totalSize += nodesExtentsDataSize;
    totalSize += trianglesDataSize;

    return totalSize;
}