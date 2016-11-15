#include "SpotLightParser.h"

#include "BinaryFile.h"

#include "SpotLight.h"

using namespace Engine1;

std::shared_ptr< SpotLight > SpotLightParser::parseBinary( std::vector< char >::const_iterator& dataIt )
{
    std::shared_ptr< SpotLight > light = std::make_shared< SpotLight >();

    light->setEnabled( BinaryFile::readBool( dataIt ) );
    light->setCastingShadows( BinaryFile::readBool( dataIt ) );
    light->setPosition( BinaryFile::readFloat3( dataIt ) );
    light->setColor( BinaryFile::readFloat3( dataIt ) );
    light->setDirection( BinaryFile::readFloat3( dataIt ) );
    light->setConeAngle( BinaryFile::readFloat( dataIt ) );

    return light;
}

void SpotLightParser::writeBinary( std::vector<char>& data, const SpotLight& light )
{
    BinaryFile::writeBool( data, light.isEnabled() );
    BinaryFile::writeBool( data, light.isCastingShadows() );
    BinaryFile::writeFloat3( data, light.getPosition() );
    BinaryFile::writeFloat3( data, light.getColor() );
    BinaryFile::writeFloat3( data, light.getDirection() );
    BinaryFile::writeFloat( data, light.getConeAngle() );
}