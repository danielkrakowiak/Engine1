#include "BlockModelParser.h"

#include "BlockMesh.h"
#include "BlockModel.h"
#include "BinaryFile.h"

std::string BlockModelParser::fileTypeIdentifier = "BLOCKMODEL";

std::shared_ptr<BlockModel> BlockModelParser::parseBinary( const std::vector<unsigned char>& data, const bool loadRecurrently )
{
	std::shared_ptr<BlockModel> model = std::make_shared<BlockModel>();

	std::vector<unsigned char>::const_iterator dataIt = data.begin();

	std::string readFileTypeIdentifier = BinaryFile::readText( dataIt, fileTypeIdentifier.size() );

	// Check file type identifier.
	if ( readFileTypeIdentifier.compare( fileTypeIdentifier ) != 0 )
		throw std::exception( "BlockModelParser::parseBinary - incorrect file type." );

	model->setMesh( BlockMesh::createFromFileInfoBinary( dataIt, loadRecurrently ) );

	const int emissiveTexturesCount = BinaryFile::readInt( dataIt );
	for ( int i = 0; i < emissiveTexturesCount; ++i )  {
		ModelTexture2D modelTexture = *ModelTexture2D::createFromBinary( dataIt, loadRecurrently );
		model->addEmissionTexture( modelTexture );
	}
	
	const int albedoTexturesCount = BinaryFile::readInt( dataIt );
	for ( int i = 0; i < albedoTexturesCount; ++i ) {
		ModelTexture2D modelTexture = *ModelTexture2D::createFromBinary( dataIt, loadRecurrently );
		model->addAlbedoTexture( modelTexture );
	}

	const int roughnessTexturesCount = BinaryFile::readInt( dataIt );
	for ( int i = 0; i < roughnessTexturesCount; ++i ) {
		ModelTexture2D modelTexture = *ModelTexture2D::createFromBinary( dataIt, loadRecurrently );
		model->addRoughnessTexture( modelTexture );
	}

	const int normalTexturesCount = BinaryFile::readInt( dataIt );
	for ( int i = 0; i < normalTexturesCount; ++i ) {
		ModelTexture2D modelTexture = *ModelTexture2D::createFromBinary( dataIt, loadRecurrently );
		model->addNormalTexture( modelTexture );
	}

	return model;
}

void BlockModelParser::writeBinary( std::vector<unsigned char>& data, const BlockModel& model )
{
	BinaryFile::writeText( data, fileTypeIdentifier );

	// Save the mesh.
	std::shared_ptr<const BlockMesh> mesh = model.getMesh( );
	if ( mesh )  {
		mesh->writeFileInfoBinary( data );
	} else {
		BlockMesh emptyMesh;
		emptyMesh.writeFileInfoBinary( data );
	}
	
	// Save the textures.
	std::vector<ModelTexture2D> textures = model.getEmissionTextures();
	BinaryFile::writeInt( data, textures.size( ) );
	for ( const ModelTexture2D& modelTexture : textures )
		modelTexture.writeBinary( data );

	textures = model.getAlbedoTextures();
	BinaryFile::writeInt( data, textures.size() );
	for ( const ModelTexture2D& modelTexture : textures )
		modelTexture.writeBinary( data );

	textures = model.getRoughnessTextures();
	BinaryFile::writeInt( data, textures.size() );
	for ( const ModelTexture2D& modelTexture : textures )
		modelTexture.writeBinary( data );

	textures = model.getNormalTextures();
	BinaryFile::writeInt( data, textures.size() );
	for ( const ModelTexture2D& modelTexture : textures )
		modelTexture.writeBinary( data );
}


