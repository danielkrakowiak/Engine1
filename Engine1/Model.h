#pragma once

#include <memory>
#include <vector>

#include "ModelTexture2D.h"

struct ID3D11Device3;

namespace Engine1
{
    class Model : public Asset
    {
        public:

        enum class TextureType
        {
            Alpha = 0,
            Emissive,
            Albedo,
            Metalness,
            Roughness,
            Normal,
            RefractiveIndex,
            COUNT
        };

        static std::string textureTypeToString( TextureType type );
        static TextureType textureFileNameToType( std::string fileName ); // Decides based on file name suffix (_A, _N etc).
        static TextureType textureNameSuffixToType( std::string nameSuffix );
        static std::string textureTypeToNameSuffix( TextureType type );

        virtual Asset::Type                                 getType() const = 0;
        virtual std::vector< std::shared_ptr<const Asset> > getSubAssets() const = 0;
        virtual std::vector< std::shared_ptr<Asset> >       getSubAssets() = 0;
        virtual void                                        swapSubAsset( std::shared_ptr<Asset> oldAsset, std::shared_ptr<Asset> newAsset ) = 0;

        virtual void saveToFile( const std::string& path ) const = 0;
        virtual void saveToMemory( std::vector<char>& data ) const = 0;

        virtual void loadCpuToGpu( ID3D11Device3& device, ID3D11DeviceContext3& deviceContext, bool reload ) = 0;
        virtual void loadGpuToCpu() = 0;
        virtual void unloadFromCpu() = 0;
        virtual void unloadFromGpu() = 0;
        virtual bool isInCpuMemory() const = 0;
        virtual bool isInGpuMemory() const = 0;

        void addTexture( const TextureType type, std::shared_ptr< Asset > texture, const int texcoordIndex = 0, const float4 colorMultiplier = float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
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
        void removeAllTextures( const TextureType type );

        int getTextureCount( const TextureType type ) const;

        std::vector< std::tuple< std::shared_ptr< Asset >, int > > getTextures( const TextureType type ) const;
        const std::vector< ModelTexture2D< unsigned char > >&  getAlphaTextures() const;
              std::vector< ModelTexture2D< unsigned char > >&  getAlphaTextures();
        const std::vector< ModelTexture2D< uchar4 > >&         getEmissiveTextures() const;
              std::vector< ModelTexture2D< uchar4 > >&         getEmissiveTextures();
        const std::vector< ModelTexture2D< uchar4 > >&	       getAlbedoTextures() const;
              std::vector< ModelTexture2D< uchar4 > >&	       getAlbedoTextures();
        const std::vector< ModelTexture2D< unsigned char > >&  getMetalnessTextures() const;
              std::vector< ModelTexture2D< unsigned char > >&  getMetalnessTextures();
        const std::vector< ModelTexture2D< unsigned char > >&  getRoughnessTextures() const;
              std::vector< ModelTexture2D< unsigned char > >&  getRoughnessTextures();
        const std::vector< ModelTexture2D< uchar4 > >&	       getNormalTextures() const;
              std::vector< ModelTexture2D< uchar4 > >&	       getNormalTextures();
        const std::vector< ModelTexture2D< unsigned char > >&  getRefractiveIndexTextures() const;
              std::vector< ModelTexture2D< unsigned char > >&  getRefractiveIndexTextures();

        std::shared_ptr< Asset > getTexture( const TextureType type, int index = 0 ) const;

        float4 getTextureColorMultiplier( const TextureType type, int index = 0 ) const;
        void   setTextureColorMultiplier( const float4& colorMul, const TextureType type, int index = 0 );

        virtual void setInterpolated( const Model& model1, const Model& model2, float ratio );

        protected:

        Model();
        Model( const Model& );
        virtual ~Model();

        std::vector< ModelTexture2D< unsigned char > > m_alphaTextures;
        std::vector< ModelTexture2D< uchar4 > >        m_emissiveTextures;
        std::vector< ModelTexture2D< uchar4 > >        m_albedoTextures;
        std::vector< ModelTexture2D< unsigned char > > m_metalnessTextures;
        std::vector< ModelTexture2D< unsigned char > > m_roughnessTextures;
        std::vector< ModelTexture2D< uchar4 > >        m_normalTextures;
        std::vector< ModelTexture2D< unsigned char > > m_refractiveIndexTextures;

        Model& operator=( const Model& ) = delete;
    };
}
