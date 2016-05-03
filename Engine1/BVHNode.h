#pragma once

#include <memory>
#include <vector>

#include "float3.h"

namespace Engine1
{
    class BVHNode 
    {
        friend class BVHTree;

	    float3 min;
	    float3 max;

        public:

        const float3& getMin() const { return min; }
        const float3& getMax() const { return max; }
	    virtual bool  isLeaf() const = 0;
    };

    class BVHInnerNode : public BVHNode 
    {
        friend class BVHTree;

	    std::unique_ptr< BVHNode > leftChild;
	    std::unique_ptr< BVHNode > rightChild;

        public: 

        const BVHNode&  getLeftChild()  const { return *leftChild; }
        const BVHNode&  getRightChild() const { return *rightChild; }
        virtual bool    isLeaf()        const { return false; }
    };

    class BVHLeafNode : public BVHNode 
    {
        friend class BVHTree;

	    std::vector< unsigned int > triangles;

        public:

        const std::vector< unsigned int >& getTriangles() const { return triangles; }
        virtual bool                       isLeaf()       const { return true; }
    };
};

