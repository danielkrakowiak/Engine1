#include "BlockModel.h"

#include "BinaryFile.h"
#include "BlockModelParser.h"

std::shared_ptr<BlockModel> BlockModel::createFromFile( const std::string& path, const FileFormat format, bool loadRecurrently )
{
	std::shared_ptr< std::vector<char> > fileData = BinaryFile::load( path );

	return createFromMemory( *fileData, format, loadRecurrently );
}

std::shared_ptr<BlockModel> BlockModel::createFromMemory( std::vector<char>& fileData, const FileFormat format, bool loadRecurrently )
{
	if ( FileFormat::BLOCKMODEL == format ) {
		return BlockModelParser::parseBinary( fileData, loadRecurrently );
	}
}

BlockModel::BlockModel( ) 
: mesh( nullptr ) 
{}


BlockModel::~BlockModel( ) 
{}

void BlockModel::saveToFile( const std::string& path )
{
	std::vector<char> data;

	BlockModelParser::writeBinary( data, *this );

	BinaryFile::save( path, data );
}

void BlockModel::loadCpuToGpu( ID3D11Device& device )
{
	if ( mesh )
		mesh->loadCpuToGpu( device );

	std::vector<ModelTexture2D> textures = getAllTextures();
	for ( ModelTexture2D& texture : textures ) {
		if ( texture.getTexture() )
			texture.getTexture()->loadCpuToGpu( device );
	}
}

void BlockModel::loadGpuToCpu()
{
	if ( mesh )
		mesh->loadGpuToCpu();

	std::vector<ModelTexture2D> textures = getAllTextures();
	for ( ModelTexture2D& texture : textures ) {
		if ( texture.getTexture() )
			texture.getTexture()->loadGpuToCpu();
	}
}

void BlockModel::unloadFromCpu()
{
	if ( mesh )
		mesh->unloadFromCpu();

	std::vector<ModelTexture2D> textures = getAllTextures();
	for ( ModelTexture2D& texture : textures ) {
		if ( texture.getTexture() )
			texture.getTexture()->unloadFromCpu();
	}
}

void BlockModel::unloadFromGpu()
{
	if ( mesh )
		mesh->unloadFromGpu();

	std::vector<ModelTexture2D> textures = getAllTextures();
	for ( ModelTexture2D& texture : textures ) {
		if ( texture.getTexture() )
			texture.getTexture()->unloadFromGpu();
	}
}

bool BlockModel::isInCpuMemory() const
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

bool BlockModel::isInGpuMemory() const
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

void BlockModel::setMesh( std::shared_ptr<BlockMesh> mesh ) {
	this->mesh = mesh;
}

std::shared_ptr<const BlockMesh> BlockModel::getMesh() const {
	return std::static_pointer_cast<const BlockMesh>( mesh );
}

std::shared_ptr<BlockMesh> BlockModel::getMesh( ) {
	return mesh;
}

void BlockModel::addEmissionTexture( ModelTexture2D& texture )
{
	emissionTextures.push_back( texture );
}

void BlockModel::addAlbedoTexture( ModelTexture2D& texture )
{
	albedoTextures.push_back( texture );
}

void BlockModel::addRoughnessTexture( ModelTexture2D& texture )
{
	roughnessTextures.push_back( texture );
}

void BlockModel::addNormalTexture( ModelTexture2D& texture )
{
	normalTextures.push_back( texture );
}

std::vector<ModelTexture2D> BlockModel::getAllTextures() const
{
	std::vector<ModelTexture2D> textures;

	textures.reserve( emissionTextures.size() + albedoTextures.size() + roughnessTextures.size() + normalTextures.size() );

	textures.insert( textures.end(),  emissionTextures.begin(),   emissionTextures.end() );
	textures.insert( textures.end( ), albedoTextures.begin( ),    albedoTextures.end( ) );
	textures.insert( textures.end( ), roughnessTextures.begin( ), roughnessTextures.end( ) );
	textures.insert( textures.end( ), normalTextures.begin( ),    normalTextures.end( ) );

	return textures;
}

std::vector<ModelTexture2D> BlockModel::getEmissionTextures( ) const {
	return std::vector<ModelTexture2D>( emissionTextures.begin( ), emissionTextures.end( ) );
}

std::vector<ModelTexture2D> BlockModel::getAlbedoTextures( ) const {
	return std::vector<ModelTexture2D>( albedoTextures.begin( ), albedoTextures.end( ) );
}

ModelTexture2D BlockModel::getAlbedoTexture( int index ) const {
	if ( index >= (int)albedoTextures.size() ) throw std::exception( "BlockModel::getAlbedoTexture: Trying to access texture at non-existing index" );

	return albedoTextures.at( index );
}

std::vector<ModelTexture2D> BlockModel::getRoughnessTextures( ) const {
	return std::vector<ModelTexture2D>( roughnessTextures.begin( ), roughnessTextures.end( ) );
}

ModelTexture2D BlockModel::getRoughnessTexture( int index ) const {
	if ( index >= (int)roughnessTextures.size( ) ) throw std::exception( "BlockModel::getRoughnessTexture: Trying to access texture at non-existing index" );

	return roughnessTextures.at( index );
}

std::vector<ModelTexture2D> BlockModel::getNormalTextures( ) const {
	return std::vector<ModelTexture2D>( normalTextures.begin( ), normalTextures.end( ) );
}

ModelTexture2D BlockModel::getNormalTexture( int index ) const {
	if ( index >= (int)normalTextures.size( ) ) throw std::exception( "BlockModel::getNormalTexture: Trying to access texture at non-existing index" );

	return normalTextures.at( index );
}
