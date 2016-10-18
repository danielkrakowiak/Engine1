#include "PointLightParser.h"

#include "BinaryFile.h"

#include "PointLight.h"

using namespace Engine1;

std::shared_ptr< PointLight > PointLightParser::parseBinary( std::vector< char >::const_iterator& dataIt )
{
    std::shared_ptr< PointLight > light = std::make_shared< PointLight >();

    light->setEnabled( BinaryFile::readBool( dataIt ) );
    light->setPosition( BinaryFile::readFloat3( dataIt ) );
    light->setColor( BinaryFile::readFloat3( dataIt ) );

    return light;
}

void PointLightParser::writeBinary( std::vector<char>& data, const PointLight& light )
{
    BinaryFile::writeBool( data, light.isEnabled() );
    BinaryFile::writeFloat3( data, light.getPosition() );
    BinaryFile::writeFloat3( data, light.getColor( ) );
}