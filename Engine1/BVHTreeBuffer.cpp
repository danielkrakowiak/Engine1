#include "BVHTreeBuffer.h"

#include <assert.h>

#include "BVHTree.h"

using namespace Engine1;

const int BVHTreeBuffer::BVH_STACK_SIZE = 32;

BVHTreeBuffer::BVHTreeBuffer( BVHTree& tree )
{
    build( tree );
}

BVHTreeBuffer::~BVHTreeBuffer()
{}

void BVHTreeBuffer::build( BVHTree& tree )
{
    int maxDepth = 0;
	countDepth( tree.getRootNode(), 0, maxDepth );
	if ( maxDepth >= BVH_STACK_SIZE )
		throw std::exception( "BVHTreeBuffer::build - input BVH tree has depth exceeding maximum supported by BVHTreeBuffer." );

	unsigned trianglesInsertIndex = 0; // Index in the triangles buffer where to place new triangles.
	unsigned nodesInsertIndex     = 0; // Index in the BVH buffer where to place new nodes.

	unsigned int triangleCount = countTriangles( tree.getRootNode() );
    m_triangles.resize( triangleCount );

	unsigned int nodeCount = countNodes( tree.getRootNode() );
	m_bvhBuffer.resize( nodeCount );

	buildBuffer( m_bvhBuffer, m_triangles, tree.getRootNode(), nodesInsertIndex, trianglesInsertIndex);

    // Check if the created buffer BVH tree is the same as the input BVH tree.
    // TODO: Should be removed later. Replaced by a unit test.
	assert( ((nodesInsertIndex == nodeCount - 1) && (trianglesInsertIndex == triangleCount)) );
}

void BVHTreeBuffer::buildBuffer( std::vector< BVHTreeBuffer::Node >& bvhBuffer, std::vector< int >& triangles, const BVHNode& node, unsigned int& nodesInsertIndex, unsigned int& trianglesInsertIndex )
{
	unsigned int currentNodesInsertIndex = nodesInsertIndex;

	bvhBuffer[ currentNodesInsertIndex ].min = node.getMin();
	bvhBuffer[ currentNodesInsertIndex ].max = node.getMax();

	// Depth first approach - left child first until completed.
	if ( !node.isLeaf() ) 
    { // Inner node.
		const BVHInnerNode& innerNode = static_cast< const BVHInnerNode& >( node );

		// Recursively build left and right child nodes.
		int idxLeft = ++nodesInsertIndex;
		buildBuffer( bvhBuffer, triangles, innerNode.getLeftChild(), nodesInsertIndex, trianglesInsertIndex);
		int idxRight = ++nodesInsertIndex;
		buildBuffer( bvhBuffer, triangles, innerNode.getRightChild(), nodesInsertIndex, trianglesInsertIndex);

		bvhBuffer[ currentNodesInsertIndex ].node.inner.childIndexLeft  = idxLeft;
		bvhBuffer[ currentNodesInsertIndex ].node.inner.childIndexRight = idxRight;
	}
	else 
    { // Leaf node.
		const BVHLeafNode& leafNode = static_cast< const BVHLeafNode& >( node );

		unsigned int triangleCount = (unsigned int)leafNode.getTriangles().size();

		bvhBuffer[ currentNodesInsertIndex ].node.leaf.triangleCount      = 0x80000000 | triangleCount;  // Set top bit = 1 to indicate that this node is a leaf node.
		bvhBuffer[ currentNodesInsertIndex ].node.leaf.firstTriangleIndex = trianglesInsertIndex;

		for ( const int triangle : leafNode.getTriangles() )
			triangles[ trianglesInsertIndex++ ] = triangle;
	}
}

unsigned int BVHTreeBuffer::countNodes(const BVHNode& node)
{
	if ( !node.isLeaf() ) 
    {
		const BVHInnerNode& innerNode = static_cast< const BVHInnerNode& >( node );
		return 1 + countNodes( innerNode.getLeftChild() ) + countNodes( innerNode.getRightChild() );
	}
    else
		return 1;
}

unsigned int BVHTreeBuffer::countTriangles(const BVHNode& node)
{
	if ( !node.isLeaf() ) 
    {
		const BVHInnerNode& innerNode = static_cast< const BVHInnerNode& >( node );
		return countTriangles( innerNode.getLeftChild() ) + countTriangles( innerNode.getRightChild() );
	}
	else {
		const BVHLeafNode& leafNode = static_cast< const BVHLeafNode& >( node );
		return (unsigned int)leafNode.getTriangles().size();
	}
}

void BVHTreeBuffer::countDepth(const BVHNode& node, int depth, int& maxDepth)
{
    if ( depth > maxDepth )
	    maxDepth = depth;

	if ( !node.isLeaf() ) {
		const BVHInnerNode& innerNode = static_cast< const BVHInnerNode& >( node );
		countDepth( innerNode.getLeftChild(),  depth + 1, maxDepth);
		countDepth( innerNode.getRightChild(), depth + 1, maxDepth);
	}
}
