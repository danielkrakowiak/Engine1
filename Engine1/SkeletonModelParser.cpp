#include "SkeletonModelParser.h"

#include <d3d11.h>

#include "SkeletonMesh.h"
#include "SkeletonModel.h"
#include "BinaryFile.h"

using namespace Engine1;

std::shared_ptr<SkeletonModel> SkeletonModelParser::parseBinary( std::vector<char>::const_iterator& dataIt, const bool loadRecurrently, ID3D11Device& device )
{
	std::shared_ptr<SkeletonModel> model = std::make_shared<SkeletonModel>( );

	// Parse mesh file info.
	std::shared_ptr<SkeletonMeshFileInfo> meshFileInfo = SkeletonMeshFileInfo::createFromMemory( dataIt );

	// Load mesh.
	std::shared_ptr< SkeletonMesh > mesh = nullptr;
	if ( loadRecurrently ) {
		std::vector< std::shared_ptr<SkeletonMesh> > loadedMeshes = SkeletonMesh::createFromFile( meshFileInfo->getPath( ), meshFileInfo->getFormat( ), meshFileInfo->getInvertZCoordinate( ), meshFileInfo->getInvertVertexWindingOrder( ), meshFileInfo->getFlipUVs( ) );

		if ( (unsigned int)meshFileInfo->getIndexInFile() < loadedMeshes.size() )
			mesh = loadedMeshes.at( meshFileInfo->getIndexInFile() );
		else
			throw std::exception( "SkeletonModelParser::parseBinary - successfully parsed mesh file info, but failed to load mesh from the file." );
	} else {
		mesh = std::make_shared< SkeletonMesh >( );
		mesh->setFileInfo( *meshFileInfo );
	}

	model->setMesh( mesh );

	const int emissiveTexturesCount = BinaryFile::readInt( dataIt );
	for ( int i = 0; i < emissiveTexturesCount; ++i ) {
		ModelTexture2D modelTexture = *ModelTexture2D::createFromMemory( dataIt, loadRecurrently, device );
		model->addEmissionTexture( modelTexture );
	}

	const int albedoTexturesCount = BinaryFile::readInt( dataIt );
	for ( int i = 0; i < albedoTexturesCount; ++i ) {
		ModelTexture2D modelTexture = *ModelTexture2D::createFromMemory( dataIt, loadRecurrently, device );
		model->addAlbedoTexture( modelTexture );
	}

	const int roughnessTexturesCount = BinaryFile::readInt( dataIt );
	for ( int i = 0; i < roughnessTexturesCount; ++i ) {
		ModelTexture2D modelTexture = *ModelTexture2D::createFromMemory( dataIt, loadRecurrently, device );
		model->addRoughnessTexture( modelTexture );
	}

	const int normalTexturesCount = BinaryFile::readInt( dataIt );
	for ( int i = 0; i < normalTexturesCount; ++i ) {
		ModelTexture2D modelTexture = *ModelTexture2D::createFromMemory( dataIt, loadRecurrently, device );
		model->addNormalTexture( modelTexture );
	}

	return model;
}

void SkeletonModelParser::writeBinary( std::vector<char>& data, const SkeletonModel& model )
{
	// Save the mesh.
	std::shared_ptr<const SkeletonMesh> mesh = model.getMesh( );
	if ( mesh ) {
		mesh->getFileInfo().saveToMemory( data );
	} else {
		SkeletonMeshFileInfo emptyFileInfo;
		emptyFileInfo.saveToMemory( data );
	}

	// Save the textures.
	std::vector<ModelTexture2D> textures = model.getEmissionTextures();
	BinaryFile::writeInt( data, (int)textures.size() );
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


