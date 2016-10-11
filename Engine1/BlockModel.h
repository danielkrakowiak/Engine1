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

        void addAlphaTexture( ModelTexture2D< unsigned char >& texture );
        void addEmissiveTexture( ModelTexture2D< uchar4 >& texture );
        void addAlbedoTexture( ModelTexture2D< uchar4 >& texture );
        void addMetalnessTexture( ModelTexture2D< unsigned char >& texture );
        void addRoughnessTexture( ModelTexture2D< unsigned char >& texture );
        void addNormalTexture( ModelTexture2D< uchar4 >& texture );
        void addRefractiveIndexTexture( ModelTexture2D< unsigned char >& texture );

        void removeAllAlphaTextures();
        void removeAllEmissiveTextures();
        void removeAllAlbedoTextures();
        void removeAllMetalnessTextures();
        void removeAllRoughnessTextures();
        void removeAllNormalTextures();
        void removeAllRefractiveIndexTextures();

        int                                             getAlphaTexturesCount() const;
        std::vector< ModelTexture2D< unsigned char > >  getAlphaTextures() const;
        ModelTexture2D< unsigned char >                 getAlphaTexture( int index = 0 ) const;

        int                                             getEmissiveTexturesCount() const;
        std::vector< ModelTexture2D< uchar4 > >         getEmissiveTextures() const;
        ModelTexture2D< uchar4 >                        getEmissiveTexture( int index = 0 ) const;

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

        int                                             getRefractiveIndexTexturesCount() const;
        std::vector< ModelTexture2D< unsigned char > >	getRefractiveIndexTextures() const;
        ModelTexture2D< unsigned char >				    getRefractiveIndexTexture( int index = 0 ) const;

        private:

        BlockModelFileInfo m_fileInfo;

        std::shared_ptr<BlockMesh> m_mesh;

        std::vector< ModelTexture2D< unsigned char > > m_alphaTextures;
        std::vector< ModelTexture2D< uchar4 > >        m_emissiveTextures;
        std::vector< ModelTexture2D< uchar4 > >        m_albedoTextures;
        std::vector< ModelTexture2D< unsigned char > > m_metalnessTextures;
        std::vector< ModelTexture2D< unsigned char > > m_roughnessTextures;
        std::vector< ModelTexture2D< uchar4 > >        m_normalTextures;
        std::vector< ModelTexture2D< unsigned char > > m_refractiveIndexTextures;

        BlockModel& operator=(const BlockModel&) = delete;
    };
}
