#include "SkeletonModel.h"


SkeletonModel::SkeletonModel( )
: mesh( nullptr ), emissionMultiplier( float3( 1.0f, 1.0f, 1.0f ) ), albedoMultiplier( float3( 1.0f, 1.0f, 1.0f ) ), roughnessMultiplier( 1.0f ), normalMultiplier( 1.0f ) {}


SkeletonModel::~SkeletonModel( ) {}

void SkeletonModel::setMesh( std::shared_ptr<SkeletonMesh>& mesh ) {
	this->mesh = mesh;
}

std::shared_ptr<const SkeletonMesh> SkeletonModel::getMesh( ) const {
	return std::static_pointer_cast<const SkeletonMesh>( mesh );
}

std::shared_ptr<SkeletonMesh> SkeletonModel::getMesh( ) {
	return mesh;
}

void SkeletonModel::addEmissionTexture( std::shared_ptr<Texture2D>& texture, int texcoordIndex ) {
	if ( texcoordIndex < 0 ) throw std::exception( "SkeletonModel::addEmissionTexture - Incorrect texcoordIndex" );

	emissionTextures.push_back( ModelTexture2D( texture, texcoordIndex ) );
}

void SkeletonModel::addAlbedoTexture( std::shared_ptr<Texture2D>& texture, int texcoordIndex ) {
	if ( texcoordIndex < 0 ) throw std::exception( "SkeletonModel::addAlbedoTexture - Incorrect texcoordIndex" );

	albedoTextures.push_back( ModelTexture2D( texture, texcoordIndex ) );
}

void SkeletonModel::addRoughnessTexture( std::shared_ptr<Texture2D>& texture, int texcoordIndex ) {
	if ( texcoordIndex < 0 ) throw std::exception( "SkeletonModel::addRoughnessTexture - Incorrect texcoordIndex" );

	roughnessTextures.push_back( ModelTexture2D( texture, texcoordIndex ) );
}

void SkeletonModel::addNormalTexture( std::shared_ptr<Texture2D>& texture, int texcoordIndex ) {
	if ( texcoordIndex < 0 ) throw std::exception( "SkeletonModel::addNormalTexture - Incorrect texcoordIndex" );

	normalTextures.push_back( ModelTexture2D( texture, texcoordIndex ) );
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