#include "PointLight.h"

#include "PointLightParser.h"
#include "BinaryFile.h"

using namespace Engine1;

std::shared_ptr<PointLight> PointLight::createFromFile( const std::string& path )
{
    std::shared_ptr< std::vector<char> > fileData = BinaryFile::load( path );

    //#TODO: check file identifier.

    return createFromMemory( fileData->cbegin() );
}

std::shared_ptr<PointLight> PointLight::createFromMemory( std::vector<char>::const_iterator dataIt )
{
    return PointLightParser::parseBinary( dataIt );
}

PointLight::PointLight()
{}

PointLight::PointLight( const float3& position, const float3& diffuseColor, const float3& specularColor ) :
Light( position, diffuseColor, specularColor )
{}

PointLight::~PointLight()
{}

Light::Type PointLight::getType() const
{
    return Light::Type::PointLight;
}

void PointLight::saveToMemory( std::vector<char>& data )
{
    PointLightParser::writeBinary( data, *this );
}

void PointLight::saveToFile( const std::string& path )
{
    std::vector<char> data;

    PointLightParser::writeBinary( data, *this );

    BinaryFile::save( path, data );
}
