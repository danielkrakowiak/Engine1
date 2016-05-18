#pragma once

#include <memory>
#include <vector>

#include "SkeletonMesh.h"
#include "ModelTexture2D.h"

#include "SkeletonModelFileInfo.h"

struct ID3D11Device;

namespace Engine1
{
    class SkeletonModel : public Asset
    {

        public:

        static std::shared_ptr<SkeletonModel> createFromFile( const std::string& path, const SkeletonModelFileInfo::Format format, bool loadRecurrently, ID3D11Device& device );
        static std::shared_ptr<SkeletonModel> createFromMemory( std::vector<char>::const_iterator dataIt, const SkeletonModelFileInfo::Format format, bool loadRecurrently, ID3D11Device& device );

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

        void loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext );
        void loadGpuToCpu();
        void unloadFromCpu();
        void unloadFromGpu();
        bool isInCpuMemory() const;
        bool isInGpuMemory() const;

        void setMesh( std::shared_ptr<SkeletonMesh> mesh );
        std::shared_ptr<const SkeletonMesh> getMesh() const;
        std::shared_ptr<SkeletonMesh> getMesh();

        void addEmissionTexture( ModelTexture2D< uchar4 >& texture );
        void addAlbedoTexture( ModelTexture2D< uchar4 >& texture );
        void addMetalnessTexture( ModelTexture2D< unsigned char >& texture );
        void addRoughnessTexture( ModelTexture2D< unsigned char >& texture );
        void addNormalTexture( ModelTexture2D< uchar4 >& texture );
        void addIndexOfRefractionTexture( ModelTexture2D< unsigned char >& texture );

        int                                             getEmissionTexturesCount() const;
        std::vector< ModelTexture2D< uchar4 > >         getEmissionTextures() const;
        ModelTexture2D< uchar4 >                        getEmissionTexture( int index = 0 ) const;

        int                                             getAlbedoTexturesCount() const;
        std::vector< ModelTexture2D< uchar4 > >	        getAlbedoTextures() const;
        ModelTexture2D< uchar4 >				        getAlbedoTexture( int index = 0 ) const;

        int                                             getMetalnessTexturesCount() const;
        std::vector< ModelTexture2D< unsigned char > >	getMetalnessTextures() const;
        ModelTexture2D< unsigned char >   				getMetalnessTexture( int index = 0 ) const;

        int                                             getRoughnessTexturesCount() const;
        std::vector< ModelTexture2D< unsigned char > >	getRoughnessTextures() const;
        ModelTexture2D< unsigned char >				    getRoughnessTexture( int index = 0 ) const;

        int                                             getNormalTexturesCount() const;
        std::vector< ModelTexture2D< uchar4 > >	        getNormalTextures() const;
        ModelTexture2D< uchar4 >				        getNormalTexture( int index = 0 ) const;

        int                                             getIndexOfRefractionTexturesCount() const;
        std::vector< ModelTexture2D< unsigned char > >	getIndexOfRefractionTextures() const;
        ModelTexture2D< unsigned char >                 getIndexOfRefractionTexture( int index = 0 ) const;

        private:

        SkeletonModelFileInfo fileInfo;

        std::shared_ptr<SkeletonMesh> mesh;

        std::vector< ModelTexture2D< uchar4 > >        emissionTextures;
        std::vector< ModelTexture2D< uchar4 > >        albedoTextures;
        std::vector< ModelTexture2D< unsigned char > > metalnessTextures;
        std::vector< ModelTexture2D< unsigned char > > roughnessTextures;
        std::vector< ModelTexture2D< uchar4 > >        normalTextures;
        std::vector< ModelTexture2D< unsigned char > > indexOfRefractionTextures;

        // Copying is not allowed.
        SkeletonModel( const SkeletonModel& )          = delete;
        SkeletonModel& operator=(const SkeletonModel&) = delete;
    };
}
