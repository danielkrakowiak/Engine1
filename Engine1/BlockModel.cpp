#include "BlockModel.h"

#include <iostream>

BlockModel::BlockModel( ) 
: mesh( nullptr ), emissionMultiplier( float3( 1.0f, 1.0f, 1.0f ) ), albedoMultiplier( float3( 1.0f, 1.0f, 1.0f ) ), roughnessMultiplier( 1.0f ), normalMultiplier( 1.0f ){}


BlockModel::~BlockModel( ) {}

void BlockModel::setMesh( std::shared_ptr<BlockMesh>& mesh ) {
	this->mesh = mesh;
}

std::shared_ptr<const BlockMesh> BlockModel::getMesh() const {
	return std::static_pointer_cast<const BlockMesh>( mesh );
}

std::shared_ptr<BlockMesh> BlockModel::getMesh( ) {
	return mesh;
}

void BlockModel::addEmissionTexture( std::shared_ptr<Texture2D>& texture, int texcoordIndex ) {
	if ( texcoordIndex < 0 ) throw std::exception( "BlockModel::addEmissionTexture - Incorrect texcoordIndex" );

	emissionTextures.push_back( ModelTexture2D( texture, texcoordIndex ) );
}

void BlockModel::addAlbedoTexture( std::shared_ptr<Texture2D>& texture, int texcoordIndex ) {
	if ( texcoordIndex < 0 ) throw std::exception( "BlockModel::addAlbedoTexture - Incorrect texcoordIndex" );

	albedoTextures.push_back( ModelTexture2D( texture, texcoordIndex ) );
}

void BlockModel::addRoughnessTexture( std::shared_ptr<Texture2D>& texture, int texcoordIndex ) {
	if ( texcoordIndex < 0 ) throw std::exception( "BlockModel::addRoughnessTexture - Incorrect texcoordIndex" );

	roughnessTextures.push_back( ModelTexture2D( texture, texcoordIndex ) );
}

void BlockModel::addNormalTexture( std::shared_ptr<Texture2D>& texture, int texcoordIndex ) {
	if ( texcoordIndex < 0 ) throw std::exception( "BlockModel::addNormalTexture - Incorrect texcoordIndex" );

	normalTextures.push_back( ModelTexture2D( texture, texcoordIndex ) );
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
