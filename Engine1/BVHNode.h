#pragma once

#include <memory>
#include <vector>

#include "float3.h"

namespace Engine1
{
    class BVHNode 
    {
        friend class BVHTree;

	    float3 m_min;
	    float3 m_max;

        public:

        const float3& getMin() const { return m_min; }
        const float3& getMax() const { return m_max; }
	    virtual bool  isLeaf() const = 0;
    };

    class BVHInnerNode : public BVHNode 
    {
        friend class BVHTree;

	    std::unique_ptr< BVHNode > m_leftChild;
	    std::unique_ptr< BVHNode > m_rightChild;

        public: 

        const BVHNode&  getLeftChild()  const { return *m_leftChild; }
        const BVHNode&  getRightChild() const { return *m_rightChild; }
        virtual bool    isLeaf()        const { return false; }
    };

    class BVHLeafNode : public BVHNode 
    {
        friend class BVHTree;

	    std::vector< unsigned int > m_triangles;

        public:

        const std::vector< unsigned int >& getTriangles() const { return m_triangles; }
        virtual bool                       isLeaf()       const { return true; }
    };
};

