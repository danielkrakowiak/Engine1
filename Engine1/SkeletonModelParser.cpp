#include "SkeletonModelParser.h"

#include "SkeletonMesh.h"
#include "SkeletonModel.h"
#include "BinaryFile.h"

std::string SkeletonModelParser::fileTypeIdentifier = "BLOCKMODEL";

std::shared_ptr<SkeletonModel> SkeletonModelParser::parseBinary( const std::vector<unsigned char>& data, const bool loadRecurrently )
{
	std::shared_ptr<SkeletonModel> model = std::make_shared<SkeletonModel>( );

	std::vector<unsigned char>::const_iterator dataIt = data.begin();

	std::string readFileTypeIdentifier = BinaryFile::readText( dataIt, fileTypeIdentifier.size() );

	// Check file type identifier.
	if ( readFileTypeIdentifier.compare( fileTypeIdentifier ) != 0 )
		throw std::exception( "BlockModelParser::parseBinary - incorrect file type." );

	model->setMesh( SkeletonMesh::createFromFileInfoBinary( dataIt, loadRecurrently ) );

	const int emissiveTexturesCount = BinaryFile::readInt( dataIt );
	for ( int i = 0; i < emissiveTexturesCount; ++i ) {
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

void SkeletonModelParser::writeBinary( std::vector<unsigned char>& data, const SkeletonModel& model )
{
	BinaryFile::writeText( data, fileTypeIdentifier );

	// Save the mesh.
	std::shared_ptr<const SkeletonMesh> mesh = model.getMesh( );
	if ( mesh ) {
		mesh->writeFileInfoBinary( data );
	} else {
		SkeletonMesh emptyMesh;
		emptyMesh.writeFileInfoBinary( data );
	}

	// Save the textures.
	std::vector<ModelTexture2D> textures = model.getEmissionTextures();
	BinaryFile::writeInt( data, textures.size() );
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


