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

	// Parse mesh file info.
	std::shared_ptr<BlockMeshFileInfo> meshFileInfo = BlockMeshFileInfo::parseBinary( dataIt );
	
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
		mesh->getFileInfo().writeBinary( data );
	} else {
		BlockMeshFileInfo emptyFileInfo;
		emptyFileInfo.writeBinary( data );
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


