#include "ModelTexture2DParser.h"

#include "ModelTexture2D.h"

#include "BinaryFile.h"

using namespace Engine1;

std::shared_ptr<ModelTexture2D> ModelTexture2DParser::parseBinary( std::vector<char>::const_iterator& dataIt, const bool loadRecurrently, ID3D11Device& device )
{
	std::shared_ptr< ModelTexture2D > modelTexture = std::make_shared< ModelTexture2D >( );

	Texture2DFileInfo fileInfo = *Texture2DFileInfo::createFromMemory( dataIt );

    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > texture;
    if ( loadRecurrently ) {
	    texture = std::make_shared< TTexture2D< TexUsage::Default, TexBind::ShaderResource, uchar4 > >
            ( device, fileInfo, true, true, true, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM );
    } else {
        texture = std::make_shared< TTexture2D< TexUsage::Default, TexBind::ShaderResource, uchar4 > >();
        texture->setFileInfo( fileInfo );
    }

	modelTexture->setTexture( texture );
	modelTexture->setTexcoordIndex( BinaryFile::readInt( dataIt ) );
	modelTexture->setColorMultiplier( BinaryFile::readFloat4( dataIt ) );

	return modelTexture;
}

void ModelTexture2DParser::writeBinary( std::vector<char>& data, const ModelTexture2D& modelTexture )
{
	const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > texture = modelTexture.getTexture();
	
	if ( texture ) {
		texture->getFileInfo().saveToMemory( data );
	} else {
		Texture2DFileInfo emptyFileInfo;
		emptyFileInfo.saveToMemory( data );
	}

	BinaryFile::writeInt( data, modelTexture.getTexcoordIndex() );
	BinaryFile::writeFloat4( data, modelTexture.getColorMultiplier() );
}
