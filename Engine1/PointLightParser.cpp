#include "PointLightParser.h"

#include "BinaryFile.h"

#include "PointLight.h"

using namespace Engine1;

std::shared_ptr< PointLight > PointLightParser::parseBinary( std::vector< char >::const_iterator& dataIt )
{
    std::shared_ptr< PointLight > light = std::make_shared< PointLight >();

    light->setEnabled( BinaryFile::readBool( dataIt ) );
    light->setCastingShadows( BinaryFile::readBool( dataIt ) );
    light->setPosition( BinaryFile::readFloat3( dataIt ) );
    light->setColor( BinaryFile::readFloat3( dataIt ) );
    light->setEmitterRadius( BinaryFile::readFloat( dataIt ) );

    return light;
}

void PointLightParser::writeBinary( std::vector<char>& data, const PointLight& light )
{
    BinaryFile::writeBool( data, light.isEnabled() );
    BinaryFile::writeBool( data, light.isCastingShadows() );
    BinaryFile::writeFloat3( data, light.getPosition() );
    BinaryFile::writeFloat3( data, light.getColor( ) );
    BinaryFile::writeFloat( data, light.getEmitterRadius() );
}