#include "BlockModel.h"

#include <d3d11.h>

#include "BinaryFile.h"
#include "BlockModelParser.h"

using namespace Engine1;

std::shared_ptr<BlockModel> BlockModel::createFromFile( const BlockModelFileInfo& fileInfo, const bool loadRecurrently, ID3D11Device& device )
{
	return createFromFile( fileInfo.getPath(), fileInfo.getFormat(), loadRecurrently, device );
}

std::shared_ptr<BlockModel> BlockModel::createFromFile( const std::string& path, const BlockModelFileInfo::Format format, const bool loadRecurrently, ID3D11Device& device )
{
	std::shared_ptr< std::vector<char> > fileData = BinaryFile::load( path );

	std::shared_ptr<BlockModel> model = createFromMemory( fileData->cbegin(), format, loadRecurrently, device );

    model->getFileInfo( ).setPath( path );
    model->getFileInfo( ).setFormat( format );

    return model;
}

std::shared_ptr<BlockModel> BlockModel::createFromMemory( std::vector<char>::const_iterator dataIt, const BlockModelFileInfo::Format format, const bool loadRecurrently, ID3D11Device& device )
{
	if ( BlockModelFileInfo::Format::BLOCKMODEL == format ) {
		return BlockModelParser::parseBinary( dataIt, loadRecurrently, device );
	}

	throw std::exception( "BlockModel::createFromMemory() - incorrect 'format' argument." );
}

BlockModel::BlockModel( ) 
: mesh( nullptr ) 
{}


BlockModel::~BlockModel( ) 
{}

Asset::Type BlockModel::getType() const
{
	return Asset::Type::BlockModel;
}

std::vector< std::shared_ptr<const Asset> > BlockModel::getSubAssets( ) const
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

std::vector< std::shared_ptr<Asset> > BlockModel::getSubAssets()
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

void BlockModel::swapSubAsset( std::shared_ptr<Asset> oldAsset, std::shared_ptr<Asset> newAsset )
{
	if ( !oldAsset )
		throw std::exception( "BlockModel::swapSubAsset - nullptr passed." );

	if ( mesh == oldAsset ) {
		std::shared_ptr<BlockMesh> newMesh = std::dynamic_pointer_cast<BlockMesh>( newAsset );

		if ( newMesh ) {
			mesh = newMesh;
			return;
		} else {
			throw std::exception( "BlockModel::swapSubAsset - tried to swap mesh with non-mesh asset." );
		}
	}

    std::vector<ModelTexture2D>* textureSets[] = { 
        &emissionTextures, 
        &albedoTextures, 
        &roughnessTextures, 
        &normalTextures 
    };

    for ( std::vector<ModelTexture2D>* textureSet : textureSets )
    {
        std::vector<ModelTexture2D>& textures = *textureSet;
        for ( ModelTexture2D& texture : textures ) {
            if ( texture.getTexture() == oldAsset ) {
                auto newTexture = std::dynamic_pointer_cast< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > >(newAsset);

                if ( newTexture ) {
                    texture.setTexture( newTexture );
                    return;
                } else {
                    throw std::exception( "BlockModel::swapSubAsset - tried to swap texture with non-texture asset." );
                }
            }
        }
    }
}

void BlockModel::setFileInfo( const BlockModelFileInfo& fileInfo )
{
	this->fileInfo = fileInfo;
}

const BlockModelFileInfo& BlockModel::getFileInfo() const
{
	return fileInfo;
}

BlockModelFileInfo& BlockModel::getFileInfo()
{
	return fileInfo;
}

void BlockModel::saveToFile( const std::string& path ) const
{
	std::vector<char> data;

	BlockModelParser::writeBinary( data, *this );

	BinaryFile::save( path, data );
}

void BlockModel::saveToMemory( std::vector<char>& data ) const
{
    BlockModelParser::writeBinary( data, *this );
}

void BlockModel::loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext )
{
	if ( mesh )
		mesh->loadCpuToGpu( device );

	std::vector<ModelTexture2D> textures = getAllTextures();
	for ( ModelTexture2D& texture : textures ) {
		if ( texture.getTexture() )
			texture.getTexture()->loadCpuToGpu( device, deviceContext );
	}
}

void BlockModel::loadGpuToCpu()
{
    throw std::exception( "BlockMesh::loadGpuToCpu - unimplemented method." );
	// #TODO: implement.

	/*if ( mesh )
		mesh->loadGpuToCpu();

	std::vector<ModelTexture2D> textures = getAllTextures();
	for ( ModelTexture2D& texture : textures ) {
		if ( texture.getTexture() )
			texture.getTexture()->loadGpuToCpu();
	}*/
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

bool BlockModel::isInGpuMemory() const
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
