#pragma once

#include "float3.h"

namespace Engine1
{
    class BoundingBox
    {
        public:

        BoundingBox();
        BoundingBox(const float3& min, const float3& max);
        ~BoundingBox();

        void set(const float3& min, const float3& max);

        float3 getMin() const;
        float3 getMax() const;
        float3 getCenter() const;

        private:

        float3 m_min;
        float3 m_max;
        float3 m_center;
    };
};

