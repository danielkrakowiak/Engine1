#include "SpotLight.h"

#include "SpotLightParser.h"
#include "BinaryFile.h"

using namespace Engine1;

const int2 SpotLight::s_shadowMapDimensions( 256, 256 );
const float SpotLight::s_shadowMapZNear( 0.1f );
const float SpotLight::s_shadowMapZFar( 5.0f );

std::shared_ptr< SpotLight > SpotLight::createFromFile( const std::string& path )
{
    std::shared_ptr< std::vector<char> > fileData = BinaryFile::load( path );

    //#TODO: check file identifier.
    std::vector<char>::const_iterator dataIt = fileData->cbegin();

    return createFromMemory( dataIt );
}

std::shared_ptr< SpotLight > SpotLight::createFromMemory( std::vector<char>::const_iterator& dataIt )
{
    return SpotLightParser::parseBinary( dataIt );
}

SpotLight::SpotLight() :
    m_direction( 0.0f, 0.0f, 1.0f ),
    m_coneAngle( 0.5f )
{}

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

void SpotLight::saveToMemory( std::vector<char>& data )
{
    SpotLightParser::writeBinary( data, *this );
}

void SpotLight::saveToFile( const std::string& path )
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