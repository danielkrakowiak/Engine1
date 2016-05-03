#include "BVHTree.h"

#include "BlockMesh.h"

using namespace Engine1;

BVHTree::BVHTree( const BlockMesh& mesh )
{
    m_rootNode = build( mesh );
}


BVHTree::~BVHTree()
{}

const BVHNode& BVHTree::getRootNode() const
{
    return *m_rootNode;
}

std::unique_ptr< BVHNode > BVHTree::build( const BlockMesh& mesh )
{
    const std::vector< float3 >& meshVertices       = mesh.getVertices();
    const std::vector< uint3 >&  meshTriangles      = mesh.getTriangles();
    const unsigned int           meshTrianglesCount = (unsigned int)meshTriangles.size();

    float3 meshMin(  FLT_MAX,  FLT_MAX,  FLT_MAX );
    float3 meshMax( -FLT_MAX, -FLT_MAX, -FLT_MAX );

    std::vector< TriangleBoundingBox > triangleBoundingBoxes;
    triangleBoundingBoxes.resize( meshTriangles.size() );

    { // Calculate bounding box for each triangle.
        for ( unsigned int i = 0; i < meshTrianglesCount; ++i )
        {
            const uint3& triangle = meshTriangles[ i ];
            const float3& vertex1 = meshVertices[ triangle.x ];
            const float3& vertex2 = meshVertices[ triangle.y ];
            const float3& vertex3 = meshVertices[ triangle.z ];

            // Calculate triangle bounding box.
            float3 triangleMin, triangleMax;
            triangleMin = min(vertex1,     vertex2);
            triangleMin = min(triangleMin, vertex3);
            triangleMax = max(vertex1,     vertex2);
            triangleMax = max(triangleMax, vertex3);

            triangleBoundingBoxes[ i ].triangleIndex = i;  //TODO: Is it really needed?
            triangleBoundingBoxes[ i ].boundingBox.set( triangleMin, triangleMax );

            // Update mesh bounding box.
            meshMin = min( meshMin, triangleMin );
            meshMax = max( meshMax, triangleMax );
        }
    }

    std::unique_ptr< BVHNode > rootNode = recursiveBuild( triangleBoundingBoxes );
    
    rootNode->min = meshMin;
    rootNode->max = meshMax;

    return rootNode;
}

std::unique_ptr< BVHNode > BVHTree::recursiveBuild( std::vector< TriangleBoundingBox >& triangles, int depth )
{
    // Terminate recursion case: 
	// If there is less than 4 triangles left, create a leaf node 
    // and create a list of the triangles contained in the node.
    if ( triangles.size() < 4 ) 
    {
		std::unique_ptr< BVHLeafNode > leafNode = std::make_unique< BVHLeafNode >();

		for ( TriangleBoundingBox& triangle : triangles)
            leafNode->triangles.push_back( triangle.triangleIndex );

        return std::move( leafNode );
	}

    // Otherwise, divide node further into smaller nodes.
	
    // Find triangles bounding box.
	float3 trianglesMin(  FLT_MAX,  FLT_MAX,  FLT_MAX );
	float3 trianglesMax( -FLT_MAX, -FLT_MAX, -FLT_MAX );

	for ( TriangleBoundingBox& triangle : triangles ) {
		trianglesMin = min( trianglesMin, triangle.boundingBox.getMin() );
		trianglesMax = max( trianglesMax, triangle.boundingBox.getMax() );
	}

    // SAH - surface area heuristic calculation for the nodes' bounding box (containing all the triangles).
    // SAH Cost = (number of triangles) * surfaceArea.
	const float3 sides = trianglesMax - trianglesMin;
	float minCost = triangles.size() * ( sides.x*sides.y + sides.y*sides.z + sides.z*sides.x );

	float        bestSplitPos                = FLT_MAX; // Best split position along chosen split axis.
    int          bestSplitAxis               = -1;      // 0 = X, 1 = Y, 2 = Z axis.
    unsigned int bestSplitLeftTriangleCount  = 0;
    unsigned int bestSplitRightTriangleCount = 0;

    // Try to split along axises X, Y, Z and check which gives minimal cost.
	for (int axis = 0; axis < 3; axis++) {  

		// We will try dividing the triangles along the current axis,
		// with split position moving from "start" to "stop", one "step" at a time.
		float splitPosStart = trianglesMin.getData()[ axis ];
        float splitPosStop  = trianglesMax.getData()[ axis ];

		// Bounding box side along this axis is too short, we must move to a different axis.
		if ( fabsf( splitPosStop - splitPosStart ) < 0.0001f)
			continue;

		// Binning: Try splitting at a uniform sampling (at equidistantly spaced planes) that gets smaller the deeper we go:
		// size of "sampling grid": 1024 (depth 0), 512 (depth 1), etc
		// each bin has size "step"
		const float splitPosStep = (splitPosStop - splitPosStart) / (1024.0f / (depth + 1.0f));

		// Try to split on different positions and check which gives minimal cost.
		for ( float testSplitPos = splitPosStart + splitPosStep; testSplitPos < splitPosStop - splitPosStep; testSplitPos += splitPosStep ) {

			// Create left and right bounding box
			float3 leftMin(  FLT_MAX,  FLT_MAX,  FLT_MAX );
			float3 leftMax( -FLT_MAX, -FLT_MAX, -FLT_MAX );

			float3 rightMin(  FLT_MAX,  FLT_MAX,  FLT_MAX );
			float3 rightMax( -FLT_MAX, -FLT_MAX, -FLT_MAX );

			unsigned int leftTriangleCount = 0, rightTriangleCount = 0;

			// Count triangles in the left and right bounding boxes and calculate their extents.
            // Needed to calculate SAH cost after split.
			for ( TriangleBoundingBox& triangle : triangles ) {

				// Triangle bounding box center position along split axis.
				const float triangleCenterPos = triangle.boundingBox.getCenter().getData()[ axis ];

				if ( triangleCenterPos < testSplitPos ) {
					leftMin = min( leftMin, triangle.boundingBox.getMin() );
					leftMax = max( leftMax, triangle.boundingBox.getMax() );
					++leftTriangleCount;
				} else {
					rightMin = min( rightMin, triangle.boundingBox.getMin() );
					rightMax = max( rightMax, triangle.boundingBox.getMax() );
					++rightTriangleCount;
				}
			}

			// Now use the Surface Area Heuristic to see if this split has a better "cost".

			// First, check if split is reasonable - bins with 0 or 1 triangles make no sense.
			if ( leftTriangleCount <= 1 || rightTriangleCount <= 1 ) 
                continue;

			// Split is reasonable - calculate SAH cost.
			const float3 leftSides  = leftMax - leftMin;
			const float3 rightSides = rightMax - rightMin;

			const float surfaceLeft  = leftSides.x*leftSides.y   + leftSides.y*leftSides.z   + leftSides.z*leftSides.x;
			const float surfaceRight = rightSides.x*rightSides.y + rightSides.y*rightSides.z + rightSides.z*rightSides.x;

			const float newCost = surfaceLeft*leftTriangleCount + surfaceRight*rightTriangleCount;

			// Keep track of cheapest split found so far.
			if ( newCost < minCost ) {
				minCost                     = newCost;
				bestSplitPos                = testSplitPos;
				bestSplitAxis               = axis;
                bestSplitLeftTriangleCount  = leftTriangleCount;
                bestSplitRightTriangleCount = rightTriangleCount;
			}
		}
	}

    // At this point we should have a split with lower cost than the original, non-split bounding box.
	// However, if we found no split to improve the cost, create a BVH leaf.
	if (bestSplitAxis == -1) {
		std::unique_ptr< BVHLeafNode > leafNode = std::make_unique< BVHLeafNode >();

		for ( TriangleBoundingBox& triangle : triangles)
            leafNode->triangles.push_back( triangle.triangleIndex );

        return std::move( leafNode );
	}

    // Otherwise, create BVH inner node with left and right child nodes using the optimal split axis and position.
    std::vector< TriangleBoundingBox > leftTriangles;
	std::vector< TriangleBoundingBox > rightTriangles;

    leftTriangles.reserve( bestSplitLeftTriangleCount );
    rightTriangles.reserve( bestSplitRightTriangleCount );

	float3 leftMin(   FLT_MAX,  FLT_MAX,  FLT_MAX );
	float3 leftMax(  -FLT_MAX, -FLT_MAX, -FLT_MAX );
	float3 rightMin(  FLT_MAX,  FLT_MAX,  FLT_MAX );
	float3 rightMax( -FLT_MAX, -FLT_MAX, -FLT_MAX );
	
	// Distribute the triangles in the left or right child nodes.
	for ( TriangleBoundingBox& triangle : triangles ) 
    {
		// Triangle bounding box center position along the best split axis.
		const float triangleCenterPos = triangle.boundingBox.getCenter().getData()[ bestSplitAxis ];

		if ( triangleCenterPos < bestSplitPos ) {
			leftTriangles.push_back( triangle );
			leftMin = min( leftMin, triangle.boundingBox.getMin() );
			leftMax = max( leftMax, triangle.boundingBox.getMax() );
		} else {
			rightTriangles.push_back( triangle );
			rightMin = min( rightMin, triangle.boundingBox.getMin() );
			rightMax = max( rightMax, triangle.boundingBox.getMax() );
		}
	}

    // Create inner node.
	std::unique_ptr< BVHInnerNode > innerNode = std::make_unique< BVHInnerNode >();

	// Recursively build the left child.
	innerNode->leftChild = recursiveBuild( leftTriangles, depth + 1);
	innerNode->leftChild->min = leftMin;
	innerNode->leftChild->max = leftMax;

	// Recursively build the right child.
	innerNode->rightChild = recursiveBuild( rightTriangles, depth + 1);
	innerNode->rightChild->min = rightMin;
	innerNode->rightChild->max = rightMax;

	return std::move( innerNode );
}
