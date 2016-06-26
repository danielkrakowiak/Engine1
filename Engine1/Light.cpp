#include "Light.h"

using namespace Engine1;

Light::Light() :
position( float3::ZERO ),
color( float3::ZERO )
{}

Light::Light( const float3& position, const float3& color ) :
position( position ),
color( color )
{}

Light::~Light( )
{}

void Light::setPosition( const float3& position )
{
    this->position = position;
}

void Light::setColor( const float3& color )
{
    this->color = color;
}

float3 Light::getPosition() const
{
    return position;
}

float3 Light::getColor() const
{
    return color;
}