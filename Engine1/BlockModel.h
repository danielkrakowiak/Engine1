#pragma once

#include <memory>
#include <vector>

#include "BlockMesh.h"
#include "ModelTexture2D.h"

class BlockModel {

	public:
	BlockModel();
	~BlockModel();

	void setMesh( std::shared_ptr<BlockMesh>& mesh );
	std::shared_ptr<const BlockMesh> getMesh() const;
	std::shared_ptr<BlockMesh> getMesh();

	void addEmissionTexture( std::shared_ptr<Texture2D>& texture, int texcoordIndex = 0 );
	void addAlbedoTexture( std::shared_ptr<Texture2D>& texture, int texcoordIndex = 0 );
	void addRoughnessTexture( std::shared_ptr<Texture2D>& texture, int texcoordIndex = 0 );
	void addNormalTexture( std::shared_ptr<Texture2D>& texture, int texcoordIndex = 0 );

	std::vector<ModelTexture2D>       getEmissionTextures() const;
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

	std::shared_ptr<BlockMesh> mesh;

	std::vector<ModelTexture2D> emissionTextures;
	std::vector<ModelTexture2D> albedoTextures;
	std::vector<ModelTexture2D> roughnessTextures;
	std::vector<ModelTexture2D> normalTextures;

	// Copying is not allowed.
	BlockModel( const BlockModel& ) = delete;
	BlockModel& operator=( const BlockModel& ) = delete;
};
