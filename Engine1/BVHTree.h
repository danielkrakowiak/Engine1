#pragma once

#include <memory>
#include <vector>

#include "BVHNode.h"
#include "BoundingBox.h"

namespace Engine1
{
    class BlockMesh;

    // Reference: https://github.com/straaljager/GPU-path-tracing-tutorial-3
    class BVHTree
    {
        struct TriangleBoundingBox
        {
            unsigned int triangleIndex;
            BoundingBox  boundingBox;
        };

        public:

        BVHTree( const BlockMesh& mesh );
        ~BVHTree();

        const BVHNode& getRootNode() const;

        private:

        std::unique_ptr< BVHNode > m_rootNode;

        std::unique_ptr< BVHNode > build( const BlockMesh& mesh );
        std::unique_ptr< BVHNode > recursiveBuild( std::vector< TriangleBoundingBox >& triangleBoundingBoxes, int depth = 0 );

    };
};

