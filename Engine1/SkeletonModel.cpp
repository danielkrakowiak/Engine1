#include "SkeletonModel.h"

#include "BinaryFile.h"
#include "SkeletonModelParser.h"

using namespace Engine1;

std::shared_ptr<SkeletonModel> SkeletonModel::createFromFile( const std::string& path, const SkeletonModelFileInfo::Format format, bool loadRecurrently )
{
	std::shared_ptr< std::vector<char> > fileData = BinaryFile::load( path );

    std::shared_ptr<SkeletonModel> model = createFromMemory( fileData->begin(), format, loadRecurrently );

    model->getFileInfo().setPath( path );
    model->getFileInfo().setFormat( format );

    return model;
}

std::shared_ptr<SkeletonModel> SkeletonModel::createFromMemory( std::vector<char>::const_iterator& dataIt, const SkeletonModelFileInfo::Format format, bool loadRecurrently )
{
	if ( SkeletonModelFileInfo::Format::SKELETONMODEL == format ) {
		return SkeletonModelParser::parseBinary( dataIt, loadRecurrently );
	}

	throw std::exception( "SkeletonModel::createFromMemory() - incorrect 'format' argument." );
}

SkeletonModel::SkeletonModel( )
: mesh( nullptr ) 
{}

SkeletonModel::~SkeletonModel( ) {}

Asset::Type SkeletonModel::getType( ) const
{
	return Asset::Type::SkeletonModel;
}

std::vector< std::shared_ptr<const Asset> > SkeletonModel::getSubAssets( ) const
{
	std::vector< std::shared_ptr<const Asset> > subAssets;

	if ( mesh )
		subAssets.push_back( mesh );

	std::vector<ModelTexture2D> textures = getAllTextures();
	for ( const ModelTexture2D& texture : textures ) {
		if ( texture.getTexture() )
			subAssets.push_back( texture.getTexture() );
	}

	return subAssets;
}

std::vector< std::shared_ptr<Asset> > SkeletonModel::getSubAssets( )
{
	std::vector< std::shared_ptr<Asset> > subAssets;

	if ( mesh )
		subAssets.push_back( mesh );

	std::vector<ModelTexture2D> textures = getAllTextures();
	for ( ModelTexture2D& texture : textures ) {
		if ( texture.getTexture() )
			subAssets.push_back( texture.getTexture() );
	}

	return subAssets;
}

void SkeletonModel::swapSubAsset( std::shared_ptr<Asset> oldAsset, std::shared_ptr<Asset> newAsset )
{
	if ( !oldAsset )
		throw std::exception( "SkeletonModel::swapSubAsset - nullptr passed." );

	if ( mesh == oldAsset ) {
		std::shared_ptr<SkeletonMesh> newMesh = std::dynamic_pointer_cast<SkeletonMesh>( newAsset );

		if ( newMesh ) {
			mesh = newMesh;
			return;
		} else {
			throw std::exception( "SkeletonModel::swapSubAsset - tried to swap mesh with non-mesh asset." );
		}
	}

	std::vector<ModelTexture2D> textures = getAllTextures();
	for ( ModelTexture2D& texture : textures ) {
		if ( texture.getTexture() == oldAsset ) {
			std::shared_ptr<Texture2D> newTexture = std::dynamic_pointer_cast<Texture2D>( newAsset );

			if ( newTexture ) {
				texture.setTexture( newTexture );
				return;
			} else {
				throw std::exception( "SkeletonModel::swapSubAsset - tried to swap texture with non-texture asset." );
			}
		}
	}
}

void SkeletonModel::setFileInfo( const SkeletonModelFileInfo& fileInfo )
{
	this->fileInfo = fileInfo;
}

const SkeletonModelFileInfo& SkeletonModel::getFileInfo( ) const
{
	return fileInfo;
}

SkeletonModelFileInfo& SkeletonModel::getFileInfo( )
{
	return fileInfo;
}

void SkeletonModel::setMesh( std::shared_ptr<SkeletonMesh> mesh ) {
	this->mesh = mesh;
}

void SkeletonModel::saveToFile( const std::string& path )
{
	std::vector<char> data;

	SkeletonModelParser::writeBinary( data, *this );

	BinaryFile::save( path, data );
}

void SkeletonModel::saveToMemory( std::vector<char>& data ) const
{
    SkeletonModelParser::writeBinary( data, *this );
}

void SkeletonModel::loadCpuToGpu( ID3D11Device& device, bool reload )
{
	if ( mesh )
		mesh->loadCpuToGpu( device, reload );

	std::vector<ModelTexture2D> textures = getAllTextures();
	for ( ModelTexture2D& texture : textures ) {
		if ( texture.getTexture() )
			texture.getTexture()->loadCpuToGpu( device, reload );
	}
}

void SkeletonModel::loadGpuToCpu( )
{
	if ( mesh )
		mesh->loadGpuToCpu();

	std::vector<ModelTexture2D> textures = getAllTextures();
	for ( ModelTexture2D& texture : textures ) {
		if ( texture.getTexture() )
			texture.getTexture()->loadGpuToCpu();
	}
}

void SkeletonModel::unloadFromCpu( )
{
	if ( mesh )
		mesh->unloadFromCpu();

	std::vector<ModelTexture2D> textures = getAllTextures();
	for ( ModelTexture2D& texture : textures ) {
		if ( texture.getTexture() )
			texture.getTexture()->unloadFromCpu();
	}
}

void SkeletonModel::unloadFromGpu( )
{
	if ( mesh )
		mesh->unloadFromGpu();

	std::vector<ModelTexture2D> textures = getAllTextures();
	for ( ModelTexture2D& texture : textures ) {
		if ( texture.getTexture() )
			texture.getTexture()->unloadFromGpu();
	}
}

bool SkeletonModel::isInCpuMemory( ) const
{
	if ( !mesh || !mesh->isInCpuMemory() )
		return false;

	const std::vector<ModelTexture2D> textures = getAllTextures();
	if ( textures.empty() )
		return false;

	for ( const ModelTexture2D& texture : textures ) {
		if ( !texture.getTexture() || !texture.getTexture()->isInCpuMemory() )
			return false;
	}

	return true;
}

bool SkeletonModel::isInGpuMemory( ) const
{
	if ( !mesh || !mesh->isInGpuMemory() )
		return false;

	const std::vector<ModelTexture2D> textures = getAllTextures();
	if ( textures.empty() )
		return false;

	for ( const ModelTexture2D& texture : textures ) {
		if ( !texture.getTexture() || !texture.getTexture()->isInGpuMemory() )
			return false;
	}

	return true;
}

std::shared_ptr<const SkeletonMesh> SkeletonModel::getMesh( ) const {
	return std::static_pointer_cast<const SkeletonMesh>( mesh );
}

std::shared_ptr<SkeletonMesh> SkeletonModel::getMesh( ) {
	return mesh;
}

void SkeletonModel::addEmissionTexture( ModelTexture2D& texture )
{
	emissionTextures.push_back( texture );
}

void SkeletonModel::addAlbedoTexture( ModelTexture2D& texture )
{
	albedoTextures.push_back( texture );
}

void SkeletonModel::addRoughnessTexture( ModelTexture2D& texture )
{
	roughnessTextures.push_back( texture );
}

void SkeletonModel::addNormalTexture( ModelTexture2D& texture )
{
	normalTextures.push_back( texture );
}

std::vector<ModelTexture2D> SkeletonModel::getAllTextures( ) const
{
	std::vector<ModelTexture2D> textures;

	textures.reserve( emissionTextures.size() + albedoTextures.size() + roughnessTextures.size() + normalTextures.size() );

	textures.insert( textures.end(), emissionTextures.begin(), emissionTextures.end() );
	textures.insert( textures.end(), albedoTextures.begin(), albedoTextures.end() );
	textures.insert( textures.end(), roughnessTextures.begin(), roughnessTextures.end() );
	textures.insert( textures.end(), normalTextures.begin(), normalTextures.end() );

	return textures;
}

std::vector<ModelTexture2D> SkeletonModel::getEmissionTextures( ) const {
	return std::vector<ModelTexture2D>( emissionTextures.begin( ), emissionTextures.end( ) );
}

std::vector<ModelTexture2D> SkeletonModel::getAlbedoTextures( ) const {
	return std::vector<ModelTexture2D>( albedoTextures.begin( ), albedoTextures.end( ) );
}

ModelTexture2D SkeletonModel::getAlbedoTexture( int index ) const {
	if ( index >= (int)albedoTextures.size( ) ) throw std::exception( "SkeletonModel::getAlbedoTexture: Trying to access texture at non-existing index" );

	return albedoTextures.at( index );
}

std::vector<ModelTexture2D> SkeletonModel::getRoughnessTextures( ) const {
	return std::vector<ModelTexture2D>( roughnessTextures.begin( ), roughnessTextures.end( ) );
}

ModelTexture2D SkeletonModel::getRoughnessTexture( int index ) const {
	if ( index >= (int)roughnessTextures.size( ) ) throw std::exception( "SkeletonModel::getRoughnessTexture: Trying to access texture at non-existing index" );

	return roughnessTextures.at( index );
}

std::vector<ModelTexture2D> SkeletonModel::getNormalTextures( ) const {
	return std::vector<ModelTexture2D>( normalTextures.begin( ), normalTextures.end( ) );
}

ModelTexture2D SkeletonModel::getNormalTexture( int index ) const {
	if ( index >= (int)normalTextures.size( ) ) throw std::exception( "SkeletonModel::getNormalTexture: Trying to access texture at non-existing index" );

	return normalTextures.at( index );
}