#include "FreeCameraParser.h"

#include "FreeCamera.h"

#include "BinaryFile.h"

using namespace Engine1;

std::shared_ptr< FreeCamera > FreeCameraParser::parseBinary( std::vector< char >::const_iterator& dataIt )
{
    const float3 cameraPosition       = BinaryFile::readFloat3( dataIt );
    const float3 cameraRotationAngles = BinaryFile::readFloat3( dataIt );
    const float  cameraFov            = BinaryFile::readFloat( dataIt );

    std::shared_ptr< FreeCamera > camera = std::make_shared< FreeCamera >( cameraPosition, cameraRotationAngles, cameraFov );

    return camera;
}

void FreeCameraParser::writeBinary( std::vector<char>& data, const FreeCamera& camera )
{
    BinaryFile::writeFloat3( data, camera.getPosition() );
    BinaryFile::writeFloat3( data, camera.m_rotationAngles );
    BinaryFile::writeFloat( data, camera.getFieldOfView() );
}