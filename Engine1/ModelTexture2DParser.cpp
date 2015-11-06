#include "ModelTexture2DParser.h"

#include "ModelTexture2D.h"
#include "BinaryFile.h"

std::shared_ptr<ModelTexture2D> ModelTexture2DParser::parseBinary( std::vector<unsigned char>::const_iterator& dataIt, const bool loadRecurrently )
{
	std::shared_ptr<ModelTexture2D> modelTexture = std::make_shared<ModelTexture2D>( );

	modelTexture->setTexture( Texture2D::createFromFileInfoBinary( dataIt, loadRecurrently ) );
	modelTexture->setTexcoordIndex( BinaryFile::readInt( dataIt ) );
	modelTexture->setColorMultiplier( BinaryFile::readFloat4( dataIt ) );

	return modelTexture;
}

void ModelTexture2DParser::writeBinary( std::vector<unsigned char>& data, const ModelTexture2D& modelTexture )
{
	const std::shared_ptr<Texture2D> texture = modelTexture.getTexture();
	
	if ( texture ) {
		texture->writeFileInfoBinary( data );
	} else {
		Texture2D emptyTexture;
		emptyTexture.writeFileInfoBinary( data );
	}

	BinaryFile::writeInt( data, modelTexture.getTexcoordIndex() );
	BinaryFile::writeFloat4( data, modelTexture.getColorMultiplier() );
}
