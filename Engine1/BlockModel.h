#pragma once

#include <memory>
#include <vector>

#include "Model.h"

#include "BlockMesh.h"
#include "ModelTexture2D.h"

#include "BlockModelFileInfo.h"

struct ID3D11Device;

namespace Engine1
{
    class BlockModel : public Model
    {

        public:

        static std::shared_ptr<BlockModel> createFromFile( const BlockModelFileInfo& fileInfo, const bool loadRecurrently, ID3D11Device& device );
        static std::shared_ptr<BlockModel> createFromFile( const std::string& path, const BlockModelFileInfo::Format format, const bool loadRecurrently, ID3D11Device& device );
        static std::shared_ptr<BlockModel> createFromMemory( std::vector<char>::const_iterator dataIt, const BlockModelFileInfo::Format format, const bool loadRecurrently, ID3D11Device& device );

        BlockModel();
        BlockModel( const BlockModel& );
        ~BlockModel();

        Asset::Type                                 getType() const;
        std::vector< std::shared_ptr<const Asset> > getSubAssets() const;
        std::vector< std::shared_ptr<Asset> >       getSubAssets();
        void                                        swapSubAsset( std::shared_ptr<Asset> oldAsset, std::shared_ptr<Asset> newAsset );

        void                      setFileInfo( const BlockModelFileInfo& fileInfo );
        const BlockModelFileInfo& getFileInfo() const;
        BlockModelFileInfo&       getFileInfo();

        void saveToFile( const std::string& path ) const;
        void saveToMemory( std::vector<char>& data ) const;

        void loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext );
        void loadGpuToCpu();
        void unloadFromCpu();
        void unloadFromGpu();
        bool isInCpuMemory() const;
        bool isInGpuMemory() const;

        void                             setMesh( std::shared_ptr<BlockMesh> mesh );
        std::shared_ptr<const BlockMesh> getMesh() const;
        std::shared_ptr<BlockMesh>       getMesh();

        private:

        BlockModelFileInfo m_fileInfo;

        std::shared_ptr<BlockMesh> m_mesh;

        BlockModel& operator=(const BlockModel&) = delete;
    };
}
