#pragma once

#include <memory>
#include <vector>

#include "SkeletonMesh.h"
#include "ModelTexture2D.h"

#include "SkeletonModelFileInfo.h"

namespace Engine1
{
    class SkeletonModel : public Asset
    {

        public:

        static std::shared_ptr<SkeletonModel> createFromFile( const std::string& path, const SkeletonModelFileInfo::Format format, bool loadRecurrently );
        static std::shared_ptr<SkeletonModel> createFromMemory( std::vector<char>::const_iterator& dataIt, const SkeletonModelFileInfo::Format format, bool loadRecurrently );

        SkeletonModel();
        ~SkeletonModel();

        Asset::Type                                 getType() const;
        std::vector< std::shared_ptr<const Asset> > getSubAssets() const;
        std::vector< std::shared_ptr<Asset> >       getSubAssets();
        void                                        swapSubAsset( std::shared_ptr<Asset> oldAsset, std::shared_ptr<Asset> newAsset );

        void                         setFileInfo( const SkeletonModelFileInfo& fileInfo );
        const SkeletonModelFileInfo& getFileInfo() const;
        SkeletonModelFileInfo&       getFileInfo();

        void saveToFile( const std::string& path );
        void saveToMemory( std::vector<char>& data ) const;

        void loadCpuToGpu( ID3D11Device& device, bool reload = false );
        void loadGpuToCpu();
        void unloadFromCpu();
        void unloadFromGpu();
        bool isInCpuMemory() const;
        bool isInGpuMemory() const;

        void setMesh( std::shared_ptr<SkeletonMesh> mesh );
        std::shared_ptr<const SkeletonMesh> getMesh() const;
        std::shared_ptr<SkeletonMesh> getMesh();

        void addEmissionTexture( ModelTexture2D& texture );
        void addAlbedoTexture( ModelTexture2D& texture );
        void addRoughnessTexture( ModelTexture2D& texture );
        void addNormalTexture( ModelTexture2D& texture );

        std::vector<ModelTexture2D>       getAllTextures() const;

        std::vector<ModelTexture2D>       getEmissionTextures() const;
        ModelTexture2D                    getEmissionTexture( int index = 0 ) const;

        std::vector<ModelTexture2D>		  getAlbedoTextures() const;
        ModelTexture2D					  getAlbedoTexture( int index = 0 ) const;

        std::vector<ModelTexture2D>		  getRoughnessTextures() const;
        ModelTexture2D					  getRoughnessTexture( int index = 0 ) const;

        std::vector<ModelTexture2D>		  getNormalTextures() const;
        ModelTexture2D					  getNormalTexture( int index = 0 ) const;

        private:

        SkeletonModelFileInfo fileInfo;

        std::shared_ptr<SkeletonMesh> mesh;

        std::vector<ModelTexture2D> emissionTextures;
        std::vector<ModelTexture2D> albedoTextures;
        std::vector<ModelTexture2D> roughnessTextures;
        std::vector<ModelTexture2D> normalTextures;

        // Copying is not allowed.
        SkeletonModel( const SkeletonModel& ) = delete;
        SkeletonModel& operator=(const SkeletonModel&) = delete;
    };
}
