#include "Light.h"

using namespace Engine1;

Light::Light() :
    m_enabled( true ),
    m_position( float3::ZERO ),
    m_color( float3::ZERO )
{}

Light::Light( const float3& position, const float3& color, const bool enabled ) :
    m_enabled( enabled ),
    m_position( position ),
    m_color( color )
{}

Light::~Light( )
{}

void Light::setEnabled( const bool enabled )
{
    m_enabled = enabled;
}

void Light::setPosition( const float3& position )
{
    m_position = position;
}

void Light::setColor( const float3& color )
{
    m_color = color;
}

bool Light::isEnabled() const
{
    return m_enabled;
}

float3 Light::getPosition() const
{
    return m_position;
}

float3 Light::getColor() const
{
    return m_color;
}