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
        friend class BVHTreeBufferParser;

        public:

        // Bounding box.
        struct NodeExtents
        {
            float3 min;
	        float3 max;
        };

		//#TODO: Optimization - should Node and NodeExtents be merged together and accessed in GPU as stuctured buffer?
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
					//#TODO: Maybe could use that bit count trick here (isLeaf : 1, triangleCount : 7). And leave it as is in HLSL.
			        unsigned int triangleCount; // If top-most bit set - leaf node, otherwise inner node.
			        unsigned int firstTriangleIndex;
		        } leaf;
	        } node;
        };

        BVHTreeBuffer( BVHTree& tree );
        BVHTreeBuffer();
        ~BVHTreeBuffer();

        const std::vector< unsigned int >&               getTriangles()    const;
        const std::vector< BVHTreeBuffer::Node >&        getNodes()        const;
        const std::vector< BVHTreeBuffer::NodeExtents >& getNodesExtents() const;
        
        // Should be called once mesh's triangles have been reordered to match the BVH tree 
        // - it doesn't match the mesh anymore and is useless after reordering.
        void clearTriangles();

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

        // Triangle indices - can be used to reorder mesh triangles to match the BVH tree.
        std::vector< unsigned int >               m_triangles;

        std::vector< BVHTreeBuffer::Node >        m_bvhNodes;
        std::vector< BVHTreeBuffer::NodeExtents > m_bvhNodesExtents;
    };
};

