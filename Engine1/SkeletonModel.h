#pragma once

#include <memory>
#include <vector>

#include "SkeletonMesh.h"
#include "ModelTexture2D.h"

class SkeletonModel {

public:
	SkeletonModel( );
	~SkeletonModel( );

	void setMesh( std::shared_ptr<SkeletonMesh>& mesh );
	std::shared_ptr<const SkeletonMesh> getMesh( ) const;
	std::shared_ptr<SkeletonMesh> getMesh( );

	void addEmissionTexture( std::shared_ptr<Texture2D>& texture, int texcoordIndex = 0 );
	void addAlbedoTexture( std::shared_ptr<Texture2D>& texture, int texcoordIndex = 0 );
	void addRoughnessTexture( std::shared_ptr<Texture2D>& texture, int texcoordIndex = 0 );
	void addNormalTexture( std::shared_ptr<Texture2D>& texture, int texcoordIndex = 0 );

	std::vector<ModelTexture2D>       getEmissionTextures( ) const;
	ModelTexture2D                    getEmissionTexture( int index = 0 ) const;

	std::vector<ModelTexture2D>		  getAlbedoTextures( ) const;
	ModelTexture2D					  getAlbedoTexture( int index = 0 ) const;

	std::vector<ModelTexture2D>		  getRoughnessTextures( ) const;
	ModelTexture2D					  getRoughnessTexture( int index = 0 ) const;

	std::vector<ModelTexture2D>		  getNormalTextures( ) const;
	ModelTexture2D					  getNormalTexture( int index = 0 ) const;

	float3 emissionMultiplier;
	float3 albedoMultiplier;
	float roughnessMultiplier;
	float normalMultiplier;

private:

	std::shared_ptr<SkeletonMesh> mesh;

	std::vector<ModelTexture2D> emissionTextures;
	std::vector<ModelTexture2D> albedoTextures;
	std::vector<ModelTexture2D> roughnessTextures;
	std::vector<ModelTexture2D> normalTextures;

	// Copying is not allowed.
	SkeletonModel( const SkeletonModel& ) = delete;
	SkeletonModel& operator=( const SkeletonModel& ) = delete;
};
