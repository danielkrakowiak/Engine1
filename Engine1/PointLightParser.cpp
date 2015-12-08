#include "PointLightParser.h"

#include "BinaryFile.h"

#include "PointLight.h"

using namespace Engine1;

std::shared_ptr<PointLight> PointLightParser::parseBinary( std::vector<char>::const_iterator& dataIt )
{
    std::shared_ptr<PointLight> light = std::make_shared<PointLight>();

    light->setPosition(      BinaryFile::readFloat3( dataIt ) );
    light->setDiffuseColor(  BinaryFile::readFloat3( dataIt ) );
    light->setSpecularColor( BinaryFile::readFloat3( dataIt ) );

    return light;
}

void PointLightParser::writeBinary( std::vector<char>& data, const PointLight& light )
{
    BinaryFile::writeFloat3( data, light.getPosition() );
    BinaryFile::writeFloat3( data, light.getDiffuseColor( ) );
    BinaryFile::writeFloat3( data, light.getSpecularColor( ) );
}