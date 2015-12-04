#include "ModelTexture2DParser.h"

#include "ModelTexture2D.h"
#include "BinaryFile.h"

using namespace Engine1;

std::shared_ptr<ModelTexture2D> ModelTexture2DParser::parseBinary( std::vector<char>::const_iterator& dataIt, const bool loadRecurrently )
{
	std::shared_ptr<ModelTexture2D> modelTexture = std::make_shared<ModelTexture2D>( );

	Texture2DFileInfo fileInfo = *Texture2DFileInfo::parseBinary( dataIt );
	std::shared_ptr<Texture2D> texture = Texture2D::createFromFile( fileInfo.getPath(), fileInfo.getFormat() );

	modelTexture->setTexture( texture );
	modelTexture->setTexcoordIndex( BinaryFile::readInt( dataIt ) );
	modelTexture->setColorMultiplier( BinaryFile::readFloat4( dataIt ) );

	return modelTexture;
}

void ModelTexture2DParser::writeBinary( std::vector<char>& data, const ModelTexture2D& modelTexture )
{
	const std::shared_ptr<Texture2D> texture = modelTexture.getTexture();
	
	if ( texture ) {
		texture->getFileInfo().writeBinary( data );
	} else {
		Texture2DFileInfo emptyFileInfo;
		emptyFileInfo.writeBinary( data );
	}

	BinaryFile::writeInt( data, modelTexture.getTexcoordIndex() );
	BinaryFile::writeFloat4( data, modelTexture.getColorMultiplier() );
}
