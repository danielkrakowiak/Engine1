#include "SpotLight.h"

#include "SpotLightParser.h"
#include "BinaryFile.h"

#include <algorithm>
#include "MathUtil.h"

using namespace Engine1;

const int2 SpotLight::s_shadowMapDimensions( 256, 256 );
const float SpotLight::s_shadowMapZNear( 0.1f );
const float SpotLight::s_shadowMapZFar( 100.0f );

std::shared_ptr< SpotLight > SpotLight::createFromFile( const std::string& path )
{
    std::shared_ptr< std::vector<char> > fileData = BinaryFile::load( path );

    //#TODO: check file identifier.
    auto       dataIt    = fileData->cbegin();
    const auto dataEndIt = fileData->cbegin();

    return createFromMemory( dataIt, dataEndIt );
}

std::shared_ptr< SpotLight > SpotLight::createFromMemory( std::vector<char>::const_iterator& dataIt, const std::vector<char>::const_iterator& dataEndIt )
{
    //#TODO: Should check if dataIt doesn't exceed dataEndIt while parsing.
    dataEndIt;

    return SpotLightParser::parseBinary( dataIt );
}

SpotLight::SpotLight() :
    m_direction( 0.0f, 0.0f, 1.0f ),
    m_coneAngle( 0.5f )
{}

SpotLight::SpotLight( const SpotLight& light ) :
    Light( light ),
    m_direction( light.m_direction ),
    m_coneAngle( light.m_coneAngle )
{
}

SpotLight::SpotLight( const float3& position, const float3& direction, const float coneAngle, const float3& color ) :
    Light( position, color ),
    m_direction( direction ),
    m_coneAngle( coneAngle )
{}

SpotLight::~SpotLight()
{}

Light::Type SpotLight::getType() const
{
    return Light::Type::SpotLight;
}

void SpotLight::saveToMemory( std::vector<char>& data ) const
{
    SpotLightParser::writeBinary( data, *this );
}

void SpotLight::saveToFile( const std::string& path ) const
{
    std::vector<char> data;

    SpotLightParser::writeBinary( data, *this );

    BinaryFile::save( path, data );
}

void SpotLight::setDirection( const float3& direction )
{
    m_direction = direction;
}

void SpotLight::setConeAngle( const float coneAngle )
{
    m_coneAngle = coneAngle;
}

void SpotLight::setShadowMap( std::shared_ptr< Texture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, float > > shadowMap )
{
    m_shadowMap = shadowMap;
}

void SpotLight::setInterpolated( const Light& light1, const Light& light2, float ratio )
{
    ratio = std::min( 1.0f, std::max( 0.0f, ratio ) );

    Light::setInterpolated( light1, light2, ratio );

    if ( light1.getType() != Light::Type::SpotLight || light2.getType() != Light::Type::SpotLight )
        return;

    auto& spotlight1 = static_cast< const SpotLight& >( light1 );
    auto& spotlight2 = static_cast< const SpotLight& >( light2 );

    // Transform directions into rotation matrices and then into quaternions, 
    // then slerp between quaternions, turn result into rotation matrix and take direction from it (3rd row).
    m_direction = float33( quat::slerp( 
        quat( MathUtil::directionToRotationMatrix( spotlight1.m_direction ) ),
        quat( MathUtil::directionToRotationMatrix( spotlight2.m_direction ) ),
        ratio
    ) ).getRow3();
    m_direction.normalize();

    m_coneAngle = MathUtil::lerp( spotlight1.m_coneAngle, spotlight2.m_coneAngle, ratio );
}

float3 SpotLight::getDirection() const
{
    return m_direction;
}

float  SpotLight::getConeAngle() const
{
    return m_coneAngle;
}

std::shared_ptr< Texture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, float > > SpotLight::getShadowMap()
{
    return m_shadowMap;
}

const std::shared_ptr< Texture2D< TexUsage::Default, TexBind::DepthStencil_ShaderResource, float > > SpotLight::getShadowMap() const
{
    return m_shadowMap;
}

float44 SpotLight::getShadowMapViewMatrix() const
{
    const float44 viewMatrix = MathUtil::lookAtTransformation( getPosition() + getDirection(), getPosition(), float3( 0.0f, 1.0f, 0.0f ) );

    return viewMatrix;
}

float44 SpotLight::getShadowMapProjectionMatrix() const
{
    // Note: Shadow map has to be a bit larger than the spot light cone
    // to ensure correct shadow blurring near the cone limits. 
    // Otherwise we would have to assume that all regions outside of spot light's cone are in shadow,
    // which causes artefacs in soft shadows.
    const float additionalFovY = getConeAngle() * getEmitterRadius() / 100.0f; // #TODO: Come up with a real algorithm for that value..

    const float fovY = getConeAngle() * 2.0f + additionalFovY;

    const float44 projectionMatrix = MathUtil::perspectiveProjectionTransformation( fovY, (float)s_shadowMapDimensions.x / (float)s_shadowMapDimensions.y, s_shadowMapZNear, s_shadowMapZFar );

    return projectionMatrix;
}