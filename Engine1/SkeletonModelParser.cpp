#include "SkeletonModelParser.h"

#include <d3d11.h>

#include "SkeletonMesh.h"
#include "SkeletonModel.h"
#include "BinaryFile.h"

#include "ModelTexture2DParser.h"

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

    const int alphaTexturesCount = BinaryFile::readInt( dataIt );
	for ( int i = 0; i < alphaTexturesCount; ++i ) {
		ModelTexture2D< unsigned char > modelTexture = *ModelTexture2DParser< unsigned char >::parseBinary( dataIt, loadRecurrently, device );
		model->addAlphaTexture( modelTexture );
	}

	const int emissiveTexturesCount = BinaryFile::readInt( dataIt );
	for ( int i = 0; i < emissiveTexturesCount; ++i ) {
		ModelTexture2D< uchar4 > modelTexture = *ModelTexture2DParser< uchar4 >::parseBinary( dataIt, loadRecurrently, device );
		model->addEmissiveTexture( modelTexture );
	}

	const int albedoTexturesCount = BinaryFile::readInt( dataIt );
	for ( int i = 0; i < albedoTexturesCount; ++i ) {
		ModelTexture2D< uchar4 > modelTexture = *ModelTexture2DParser< uchar4 >::parseBinary( dataIt, loadRecurrently, device );
		model->addAlbedoTexture( modelTexture );
	}

    const int metalnessTexturesCount = BinaryFile::readInt( dataIt );
	for ( int i = 0; i < metalnessTexturesCount; ++i ) {
		ModelTexture2D< unsigned char > modelTexture = *ModelTexture2DParser< unsigned char >::parseBinary( dataIt, loadRecurrently, device );
		model->addMetalnessTexture( modelTexture );
	}

	const int roughnessTexturesCount = BinaryFile::readInt( dataIt );
	for ( int i = 0; i < roughnessTexturesCount; ++i ) {
		ModelTexture2D< unsigned char > modelTexture = *ModelTexture2DParser< unsigned char >::parseBinary( dataIt, loadRecurrently, device );
		model->addRoughnessTexture( modelTexture );
	}

	const int normalTexturesCount = BinaryFile::readInt( dataIt );
	for ( int i = 0; i < normalTexturesCount; ++i ) {
		ModelTexture2D< uchar4 > modelTexture = *ModelTexture2DParser< uchar4 >::parseBinary( dataIt, loadRecurrently, device );
		model->addNormalTexture( modelTexture );
	}

    const int indexOfRefractionTexturesCount = BinaryFile::readInt( dataIt );
	for ( int i = 0; i < indexOfRefractionTexturesCount; ++i ) {
		ModelTexture2D< unsigned char > modelTexture = *ModelTexture2DParser< unsigned char >::parseBinary( dataIt, loadRecurrently, device );
		model->addRefractiveIndexTexture( modelTexture );
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

    std::vector< ModelTexture2D< unsigned char > > texturesU1;
    std::vector< ModelTexture2D< uchar4 > >        texturesU4;

	// Save the textures.
    texturesU1 = model.getAlphaTextures();
	BinaryFile::writeInt( data, (int)texturesU1.size() );
	for ( const ModelTexture2D< unsigned char >& modelTexture : texturesU1 )
		ModelTexture2DParser< unsigned char >::writeBinary( data, modelTexture );

	texturesU4 = model.getEmissiveTextures();
	BinaryFile::writeInt( data, (int)texturesU4.size() );
	for ( const ModelTexture2D< uchar4 >& modelTexture : texturesU4 )
		ModelTexture2DParser< uchar4 >::writeBinary( data, modelTexture );

	texturesU4 = model.getAlbedoTextures();
	BinaryFile::writeInt( data, (int)texturesU4.size() );
	for ( const ModelTexture2D< uchar4 >& modelTexture : texturesU4 )
		ModelTexture2DParser< uchar4 >::writeBinary( data, modelTexture );

    texturesU1 = model.getMetalnessTextures();
	BinaryFile::writeInt( data, (int)texturesU1.size() );
	for ( const ModelTexture2D< unsigned char >& modelTexture : texturesU1 )
		ModelTexture2DParser< unsigned char >::writeBinary( data, modelTexture );

	texturesU1 = model.getRoughnessTextures();
	BinaryFile::writeInt( data, (int)texturesU1.size() );
	for ( const ModelTexture2D< unsigned char >& modelTexture : texturesU1 )
		ModelTexture2DParser< unsigned char >::writeBinary( data, modelTexture );

	texturesU4 = model.getNormalTextures();
	BinaryFile::writeInt( data, (int)texturesU4.size() );
	for ( const ModelTexture2D< uchar4 >& modelTexture : texturesU4 )
		ModelTexture2DParser< uchar4 >::writeBinary( data, modelTexture );

    texturesU1 = model.getRefractiveIndexTextures();
	BinaryFile::writeInt( data, (int)texturesU1.size() );
	for ( const ModelTexture2D< unsigned char >& modelTexture : texturesU1 )
		ModelTexture2DParser< unsigned char >::writeBinary( data, modelTexture );
}


