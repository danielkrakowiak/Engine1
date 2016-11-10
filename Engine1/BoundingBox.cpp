#include "BoundingBox.h"

using namespace Engine1;

BoundingBox::BoundingBox() :
    m_min( float3::ZERO ),
    m_max( float3::ZERO ),
    m_center( float3::ZERO)
{}

BoundingBox::BoundingBox(const float3& min, const float3& max)
{
    set(min, max);
}

BoundingBox::~BoundingBox()
{}

void BoundingBox::set(const float3& min, const float3& max)
{
    m_min = min;
    m_max = max;
    m_center = (min + max) * 0.5f;
}

float3 BoundingBox::getMin() const
{
    return m_min;
}

float3 BoundingBox::getMax() const
{
    return m_max;
}

float3 BoundingBox::getCenter() const
{
    return m_center;
}
