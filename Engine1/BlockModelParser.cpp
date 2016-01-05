#include "BlockModelParser.h"

#include "BlockMesh.h"
#include "BlockModel.h"
#include "BinaryFile.h"

using namespace Engine1;

std::shared_ptr<BlockModel> BlockModelParser::parseBinary( std::vector<char>::const_iterator& dataIt, const bool loadRecurrently )
{
	std::shared_ptr<BlockModel> model = std::make_shared<BlockModel>();

	// Parse mesh file info.
	std::shared_ptr<BlockMeshFileInfo> meshFileInfo = BlockMeshFileInfo::createFromMemory( dataIt );
	
	// Load mesh.
	std::shared_ptr< BlockMesh > mesh = nullptr;
	if ( loadRecurrently )  {
		std::vector< std::shared_ptr<BlockMesh> > loadedMeshes = BlockMesh::createFromFile( meshFileInfo->getPath( ), meshFileInfo->getFormat( ), meshFileInfo->getInvertZCoordinate( ), meshFileInfo->getInvertVertexWindingOrder( ), meshFileInfo->getFlipUVs( ) );

		if ( (unsigned int)meshFileInfo->getIndexInFile( ) < loadedMeshes.size( ) )
			mesh = loadedMeshes.at( meshFileInfo->getIndexInFile( ) );
		else
			throw std::exception( "BlockModelParser::parseBinary - successfully parsed mesh file info, but failed to load mesh from the file." );
	} else  {
		mesh = std::make_shared< BlockMesh >();
		mesh->setFileInfo( *meshFileInfo );
	}

	model->setMesh( mesh );

	const int emissiveTexturesCount = BinaryFile::readInt( dataIt );
	for ( int i = 0; i < emissiveTexturesCount; ++i )  {
		ModelTexture2D modelTexture = *ModelTexture2D::createFromMemory( dataIt, loadRecurrently );
		model->addEmissionTexture( modelTexture );
	}
	
	const int albedoTexturesCount = BinaryFile::readInt( dataIt );
	for ( int i = 0; i < albedoTexturesCount; ++i ) {
		ModelTexture2D modelTexture = *ModelTexture2D::createFromMemory( dataIt, loadRecurrently );
		model->addAlbedoTexture( modelTexture );
	}

	const int roughnessTexturesCount = BinaryFile::readInt( dataIt );
	for ( int i = 0; i < roughnessTexturesCount; ++i ) {
		ModelTexture2D modelTexture = *ModelTexture2D::createFromMemory( dataIt, loadRecurrently );
		model->addRoughnessTexture( modelTexture );
	}

	const int normalTexturesCount = BinaryFile::readInt( dataIt );
	for ( int i = 0; i < normalTexturesCount; ++i ) {
		ModelTexture2D modelTexture = *ModelTexture2D::createFromMemory( dataIt, loadRecurrently );
		model->addNormalTexture( modelTexture );
	}

	return model;
}

void BlockModelParser::writeBinary( std::vector<char>& data, const BlockModel& model )
{
	// Save the mesh.
	std::shared_ptr<const BlockMesh> mesh = model.getMesh( );
	if ( mesh )  {
		mesh->getFileInfo().saveToMemory( data );
	} else {
		BlockMeshFileInfo emptyFileInfo;
		emptyFileInfo.saveToMemory( data );
	}
	
	// Save the textures.
	std::vector<ModelTexture2D> textures = model.getEmissionTextures();
	BinaryFile::writeInt( data, (int)textures.size( ) );
	for ( const ModelTexture2D& modelTexture : textures )
		modelTexture.saveToMemory( data );

	textures = model.getAlbedoTextures();
	BinaryFile::writeInt( data, (int)textures.size() );
	for ( const ModelTexture2D& modelTexture : textures )
		modelTexture.saveToMemory( data );

	textures = model.getRoughnessTextures();
	BinaryFile::writeInt( data, (int)textures.size() );
	for ( const ModelTexture2D& modelTexture : textures )
		modelTexture.saveToMemory( data );

	textures = model.getNormalTextures();
	BinaryFile::writeInt( data, (int)textures.size() );
	for ( const ModelTexture2D& modelTexture : textures )
		modelTexture.saveToMemory( data );
}


