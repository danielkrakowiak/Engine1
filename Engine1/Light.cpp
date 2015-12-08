#include "Light.h"

using namespace Engine1;

Light::Light() :
position( float3::ZERO ),
diffuseColor( float3::ZERO ),
specularColor( float3::ZERO )
{}

Light::Light( const float3& position, const float3& diffuseColor, const float3& specularColor ) :
position( position ),
diffuseColor( diffuseColor ),
specularColor( specularColor )
{}

Light::~Light( )
{}

void Light::setPosition( const float3& position )
{
    this->position = position;
}

void Light::setDiffuseColor( const float3& diffuseColor )
{
    this->diffuseColor = diffuseColor;
}

void Light::setSpecularColor( const float3& specularColor )
{
    this->specularColor = specularColor;
}

float3 Light::getPosition() const
{
    return position;
}

float3 Light::getDiffuseColor() const
{
    return diffuseColor;
}

float3 Light::getSpecularColor() const
{
    return specularColor;
}
