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
        struct Node 
        {
	        // Bounding box.
	        float3 min;
	        float3 max;

	        // Parameters for leaf nodes and inner nodes occupy same space (union) to save memory.
	        // Top bit discriminates between leaf node and inner node.
	        union 
            {
		        // inner node - stores indexes to array of CacheFriendlyBVHNode
		        struct {
			        unsigned int childIndexLeft;
			        unsigned int childIndexRight;
		        } inner;
		        // leaf node: stores triangle count and starting index in triangle list
		        struct {
			        unsigned int triangleCount; // If top-most bit set - leaf node, otherwise inner node.
			        unsigned int firstTriangleIndex;
		        } leaf;
	        } node;
        };

        public:

        BVHTreeBuffer( BVHTree& tree );
        ~BVHTreeBuffer();

        private:

        static const int BVH_STACK_SIZE;

        void build( BVHTree& tree );
        void buildBuffer( std::vector< Node >& bvhBuffer, std::vector< int >& triangles, const BVHNode& node, unsigned int& idxBoxes, unsigned int& idxTriList );

        // Recursively count bounding boxes.
        unsigned int countNodes( const BVHNode& node );

        // Recursively count triangles.
        unsigned int countTriangles( const BVHNode& node );

        // Recursively count depth.
        void countDepth( const BVHNode& node, int depth, int& maxDepth );

        std::vector< int >                 m_triangles;
        std::vector< BVHTreeBuffer::Node > m_bvhBuffer;
    };
};

