#include "Light.h"

using namespace Engine1;

Light::Light() :
    m_position( float3::ZERO ),
    m_color( float3::ZERO )
{}

Light::Light( const float3& position, const float3& color ) :
    m_position( position ),
    m_color( color )
{}

Light::~Light( )
{}

void Light::setPosition( const float3& position )
{
    this->m_position = position;
}

void Light::setColor( const float3& color )
{
    this->m_color = color;
}

float3 Light::getPosition() const
{
    return m_position;
}

float3 Light::getColor() const
{
    return m_color;
}