#pragma once

#include <memory>
#include <vector>

#include "BlockMesh.h"
#include "ModelTexture2D.h"

#include "BlockModelFileInfo.h"

struct ID3D11Device;

namespace Engine1
{
    class BlockModel : public Asset
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

        void addEmissionTexture( ModelTexture2D< uchar4 >& texture );
        void addAlbedoTexture( ModelTexture2D< uchar4 >& texture );
        void addMetalnessTexture( ModelTexture2D< unsigned char >& texture );
        void addRoughnessTexture( ModelTexture2D< unsigned char >& texture );
        void addNormalTexture( ModelTexture2D< uchar4 >& texture );
        void addIndexOfRefractionTexture( ModelTexture2D< unsigned char >& texture );

        void removeAllEmissionTextures();
        void removeAllAlbedoTextures();
        void removeAllMetalnessTextures();
        void removeAllRoughnessTextures();
        void removeAllNormalTextures();
        void removeAllIndexOfRefractionTextures();

        int                                             getEmissionTexturesCount() const;
        std::vector< ModelTexture2D< uchar4 > >         getEmissionTextures() const;
        ModelTexture2D< uchar4 >                        getEmissionTexture( int index = 0 ) const;

        int                                             getAlbedoTexturesCount() const;
        std::vector< ModelTexture2D< uchar4 > >	        getAlbedoTextures() const;
        ModelTexture2D< uchar4 >				        getAlbedoTexture( int index = 0 ) const;

        int                                             getMetalnessTexturesCount() const;
        std::vector< ModelTexture2D< unsigned char > >	getMetalnessTextures() const;
        ModelTexture2D< unsigned char >				    getMetalnessTexture( int index = 0 ) const;

        int                                             getRoughnessTexturesCount() const;
        std::vector< ModelTexture2D< unsigned char > >	getRoughnessTextures() const;
        ModelTexture2D< unsigned char >				    getRoughnessTexture( int index = 0 ) const;

        int                                             getNormalTexturesCount() const;
        std::vector< ModelTexture2D< uchar4 > >	        getNormalTextures() const;
        ModelTexture2D< uchar4 >				        getNormalTexture( int index = 0 ) const;

        int                                             getIndexOfRefractionTexturesCount() const;
        std::vector< ModelTexture2D< unsigned char > >	getIndexOfRefractionTextures() const;
        ModelTexture2D< unsigned char >				    getIndexOfRefractionTexture( int index = 0 ) const;

        private:

        BlockModelFileInfo fileInfo;

        std::shared_ptr<BlockMesh> mesh;

        std::vector< ModelTexture2D< uchar4 > >        emissionTextures;
        std::vector< ModelTexture2D< uchar4 > >        albedoTextures;
        std::vector< ModelTexture2D< unsigned char > > metalnessTextures;
        std::vector< ModelTexture2D< unsigned char > > roughnessTextures;
        std::vector< ModelTexture2D< uchar4 > >        normalTextures;
        std::vector< ModelTexture2D< unsigned char > > indexOfRefractionTextures;

        BlockModel& operator=(const BlockModel&) = delete;
    };
}
