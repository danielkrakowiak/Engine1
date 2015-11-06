#include "SkeletonModel.h"

#include "BinaryFile.h"
#include "SkeletonModelParser.h"

std::shared_ptr<SkeletonModel> SkeletonModel::createFromFile( const std::string& path, const FileFormat format, bool loadRecurrently )
{
	std::shared_ptr< std::vector<unsigned char> > fileData = BinaryFile::load( path );

	return createFromMemory( *fileData, format, loadRecurrently );
}

std::shared_ptr<SkeletonModel> SkeletonModel::createFromMemory( std::vector<unsigned char>& fileData, const FileFormat format, bool loadRecurrently )
{
	if ( FileFormat::SKELETONMODEL == format ) {
		return SkeletonModelParser::parseBinary( fileData, loadRecurrently );
	}
}

SkeletonModel::SkeletonModel( )
: mesh( nullptr ) 
{}

SkeletonModel::~SkeletonModel( ) {}

void SkeletonModel::setMesh( std::shared_ptr<SkeletonMesh> mesh ) {
	this->mesh = mesh;
}

void SkeletonModel::saveToFile( const std::string& path )
{
	std::vector<unsigned char> data;

	SkeletonModelParser::writeBinary( data, *this );

	BinaryFile::save( path, data );
}

void SkeletonModel::loadCpuToGpu( ID3D11Device& device )
{
	if ( mesh )
		mesh->loadCpuToGpu( device );

	std::vector<ModelTexture2D> textures = getAllTextures();
	for ( ModelTexture2D& texture : textures ) {
		if ( texture.getTexture() )
			texture.getTexture()->loadCpuToGpu( device );
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
	if ( mesh && !mesh->isInCpuMemory() )
		return false;

	const std::vector<ModelTexture2D> textures = getAllTextures();
	for ( const ModelTexture2D& texture : textures ) {
		if ( texture.getTexture() && !texture.getTexture()->isInCpuMemory() )
			return false;
	}

	return true;
}

bool SkeletonModel::isInGpuMemory( ) const
{
	if ( mesh && !mesh->isInGpuMemory() )
		return false;

	const std::vector<ModelTexture2D> textures = getAllTextures();
	for ( const ModelTexture2D& texture : textures ) {
		if ( texture.getTexture() && !texture.getTexture()->isInGpuMemory() )
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