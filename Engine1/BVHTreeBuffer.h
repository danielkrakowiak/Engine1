#pragma once

#include <vector>

#include "float3.h"

namespace Engine1
{
    class BVHTree;
    class BVHNode;

    // BVH (Bounding Volume Hierarchy) tree stored in contiguous memory (vector) for faster access.
    // Can be sent to GPU and used for ray traversal or can be also used by the CPU.
    class BVHTreeBuffer
    {
        public:

        // Bounding box.
        struct NodeExtents
        {
            float3 min;
	        float3 max;
        };

        struct Node 
        {
	        // Parameters for leaf nodes and inner nodes occupy same space (union) to save memory.
	        // Top bit discriminates between leaf node and inner node.
	        union 
            {
		        struct {
			        unsigned int childIndexLeft;
			        unsigned int childIndexRight;
		        } inner;
		        struct {
			        unsigned int triangleCount; // If top-most bit set - leaf node, otherwise inner node.
			        unsigned int firstTriangleIndex;
		        } leaf;
	        } node;
        };

        BVHTreeBuffer( BVHTree& tree );
        ~BVHTreeBuffer();

        const std::vector< unsigned int >&               getTriangles()    const;
        const std::vector< BVHTreeBuffer::Node >&        getNodes()        const;
        const std::vector< BVHTreeBuffer::NodeExtents >& getNodesExtents() const;

        private:

        static const int BVH_STACK_SIZE;

        void build( BVHTree& tree );
        void buildBuffer( std::vector< Node >& bvhNodes, std::vector< BVHTreeBuffer::NodeExtents >& bvhNodesExtents, std::vector< unsigned int >& triangles, 
                          const BVHNode& node, unsigned int& nodesInsertIndex, unsigned int& trianglesInsertIndex );

        // Recursively count bounding boxes.
        unsigned int countNodes( const BVHNode& node );

        // Recursively count triangles.
        unsigned int countTriangles( const BVHNode& node );

        // Recursively count depth.
        void countDepth( const BVHNode& node, int depth, int& maxDepth );

        std::vector< unsigned int >               m_triangles;
        std::vector< BVHTreeBuffer::Node >        m_bvhNodes;
        std::vector< BVHTreeBuffer::NodeExtents > m_bvhNodesExtents;
    };
};

