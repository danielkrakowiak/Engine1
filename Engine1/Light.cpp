#include "Light.h"

using namespace Engine1;

Light::Light() :
    m_enabled( true ),
    m_castingShadows( true ),
    m_position( float3::ZERO ),
    m_color( float3::ZERO ),
    m_emitterRadius( 0.0f )
{}

Light::Light( const Light& light ) :
    m_enabled( light.m_enabled ),
    m_castingShadows( light.m_castingShadows ),
    m_position( light.m_position ),
    m_color( light.m_color ),
    m_emitterRadius( light.m_emitterRadius )
{}

Light::Light( const float3& position, const float3& color, const bool enabled, const bool castingShadows, const float emitterRadius ) :
    m_enabled( enabled ),
    m_castingShadows( castingShadows ),
    m_position( position ),
    m_color( color ),
    m_emitterRadius( emitterRadius )
{}

Light::~Light( )
{}

void Light::setEnabled( const bool enabled )
{
    m_enabled = enabled;
}

void Light::setCastingShadows( const bool castingShadows )
{
    m_castingShadows = castingShadows;
}

void Light::setPosition( const float3& position )
{
    m_position = position;
}

void Light::setColor( const float3& color )
{
    m_color = color;
}

void Light::setEmitterRadius( const float emitterRadius )
{
    m_emitterRadius = emitterRadius;
}

bool Light::isEnabled() const
{
    return m_enabled;
}

bool Light::isCastingShadows() const
{
    return m_castingShadows;
}

float3 Light::getPosition() const
{
    return m_position;
}

float3 Light::getColor() const
{
    return m_color;
}

float Light::getEmitterRadius() const
{
    return m_emitterRadius;
}