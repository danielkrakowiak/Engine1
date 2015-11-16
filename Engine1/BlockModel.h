#pragma once

#include <memory>
#include <vector>

#include "BlockMesh.h"
#include "ModelTexture2D.h"

#include "BlockModelFileInfo.h"

class BlockModel : public Asset {

	public:

	static std::shared_ptr<BlockModel> createFromFile( const BlockModelFileInfo& fileInfo, const bool loadRecurrently );
	static std::shared_ptr<BlockModel> createFromFile( const std::string& path, const BlockModelFileInfo::Format format, const bool loadRecurrently );
	static std::shared_ptr<BlockModel> createFromMemory( const std::vector<char>& fileData, const BlockModelFileInfo::Format format, const bool loadRecurrently );
	
	BlockModel();
	~BlockModel();

	Asset::Type                                 getType( ) const;
	std::vector< std::shared_ptr<const Asset> > getSubAssets( ) const;
	std::vector< std::shared_ptr<Asset> >       getSubAssets();
	void                                        swapSubAsset( std::shared_ptr<Asset> oldAsset, std::shared_ptr<Asset> newAsset );

	void                      setFileInfo( const BlockModelFileInfo& fileInfo );
	const BlockModelFileInfo& getFileInfo() const;
	BlockModelFileInfo&       getFileInfo();

	void saveToFile( const std::string& path );

	void loadCpuToGpu( ID3D11Device& device );
	void loadGpuToCpu();
	void unloadFromCpu();
	void unloadFromGpu();
	bool isInCpuMemory() const;
	bool isInGpuMemory() const;

	void                             setMesh( std::shared_ptr<BlockMesh> mesh );
	std::shared_ptr<const BlockMesh> getMesh() const;
	std::shared_ptr<BlockMesh>       getMesh();

	void addEmissionTexture( ModelTexture2D& texture );
	void addAlbedoTexture( ModelTexture2D& texture );
	void addRoughnessTexture( ModelTexture2D& texture );
	void addNormalTexture( ModelTexture2D& texture );

	std::vector<ModelTexture2D>       getAllTextures( ) const;

	std::vector<ModelTexture2D>       getEmissionTextures() const;
	ModelTexture2D                    getEmissionTexture( int index = 0 ) const;

	std::vector<ModelTexture2D>		  getAlbedoTextures( ) const;
	ModelTexture2D					  getAlbedoTexture( int index = 0 ) const;
	
	std::vector<ModelTexture2D>		  getRoughnessTextures( ) const;
	ModelTexture2D					  getRoughnessTexture( int index = 0 ) const;
	
	std::vector<ModelTexture2D>		  getNormalTextures( ) const;
	ModelTexture2D					  getNormalTexture( int index = 0 ) const;

	private:

	BlockModelFileInfo fileInfo;

	std::shared_ptr<BlockMesh> mesh;

	std::vector<ModelTexture2D> emissionTextures;
	std::vector<ModelTexture2D> albedoTextures;
	std::vector<ModelTexture2D> roughnessTextures;
	std::vector<ModelTexture2D> normalTextures;

	// Copying is not allowed.
	BlockModel( const BlockModel& ) = delete;
	BlockModel& operator=( const BlockModel& ) = delete;
};
