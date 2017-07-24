#pragma once

#include <memory>
#include <vector>

#include "Model.h"
#include "SkeletonMesh.h"
#include "ModelTexture2D.h"

#include "SkeletonModelFileInfo.h"

struct ID3D11Device3;

namespace Engine1
{
    class SkeletonModel : public Model
    {

        public:

        static std::shared_ptr<SkeletonModel> createFromFile( const SkeletonModelFileInfo& fileInfo, const bool loadRecurrently, ID3D11Device3& device );
        static std::shared_ptr<SkeletonModel> createFromFile( const std::string& path, const SkeletonModelFileInfo::Format format, bool loadRecurrently, ID3D11Device3& device );
        static std::shared_ptr<SkeletonModel> createFromMemory( std::vector<char>::const_iterator dataIt, const SkeletonModelFileInfo::Format format, bool loadRecurrently, ID3D11Device3& device );

        SkeletonModel();
        SkeletonModel( const SkeletonModel& );
        ~SkeletonModel();

        Asset::Type                                 getType() const;
        std::vector< std::shared_ptr<const Asset> > getSubAssets() const;
        std::vector< std::shared_ptr<Asset> >       getSubAssets();
        void                                        swapSubAsset( std::shared_ptr<Asset> oldAsset, std::shared_ptr<Asset> newAsset );

        void                         setFileInfo( const SkeletonModelFileInfo& fileInfo );
        const SkeletonModelFileInfo& getFileInfo() const;
        SkeletonModelFileInfo&       getFileInfo();

        void saveToFile( const std::string& path ) const;
        void saveToMemory( std::vector<char>& data ) const;

        void loadCpuToGpu( ID3D11Device3& device, ID3D11DeviceContext3& deviceContext, bool reload = false );
        void loadGpuToCpu();
        void unloadFromCpu();
        void unloadFromGpu();
        bool isInCpuMemory() const;
        bool isInGpuMemory() const;

        void setMesh( std::shared_ptr<SkeletonMesh> mesh );
        std::shared_ptr<const SkeletonMesh> getMesh() const;
        std::shared_ptr<SkeletonMesh> getMesh();

        private:

        SkeletonModelFileInfo m_fileInfo;

        std::shared_ptr<SkeletonMesh> m_mesh;

        SkeletonModel& operator=(const SkeletonModel&) = delete;
    };
}
