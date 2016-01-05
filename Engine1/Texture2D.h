#pragma once

#include <string>
#include <vector>
#include <memory>
#include <wrl.h>

#include "FreeImagePlus.h"

#include "int2.h"

#include "Asset.h"
#include "Texture2DFileInfo.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

namespace Engine1
{
    class Texture2D : public Asset
    {

        public:

        static std::shared_ptr<Texture2D> createFromFile( const Texture2DFileInfo& fileInfo );
        static std::shared_ptr<Texture2D> createFromFile( const std::string& path, const Texture2DFileInfo::Format format );
        static std::shared_ptr<Texture2D> createFromMemory( std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt, const Texture2DFileInfo::Format format );

        public:

        Texture2D();
        ~Texture2D();

        Asset::Type                                 getType() const;
        std::vector< std::shared_ptr<const Asset> > getSubAssets() const;
        std::vector< std::shared_ptr<Asset> >       getSubAssets();
        void                                        swapSubAsset( std::shared_ptr<Asset> oldAsset, std::shared_ptr<Asset> newAsset );

        void                     setFileInfo( const Texture2DFileInfo& fileInfo );
        const Texture2DFileInfo& getFileInfo() const;
        Texture2DFileInfo&       getFileInfo();

        virtual void loadCpuToGpu( ID3D11Device& device, bool reload = false );
        virtual void loadGpuToCpu();
        virtual void unloadFromCpu();
        virtual void unloadFromGpu();
        virtual bool isInCpuMemory() const;
        virtual bool isInGpuMemory() const;

        virtual void generateMipMapsOnCpu();
        virtual void generateMipMapsOnGpu( ID3D11DeviceContext& deviceContext );
        virtual void removeMipMapsOnCpu();
        virtual bool hasMipMapsOnCpu()      const;
        virtual bool hasMipMapsOnGpu()     const;
        virtual int  getMipMapCountOnCpu()  const;
        virtual int  getMipMapCountOnGpu() const;

        std::vector<unsigned char>&       getData( unsigned int mipMapLevel = 0 );
        const std::vector<unsigned char>& getData( unsigned int mipMapLevel = 0 ) const;

        virtual ID3D11Texture2D*          getTexture() const;
        virtual ID3D11ShaderResourceView* getShaderResource() const;

        virtual int getBytesPerPixel() const;
        virtual int getWidth( unsigned int mipMapLevel = 0 ) const;
        virtual int getHeight( unsigned int mipMapLevel = 0 ) const;
        virtual int getSize( unsigned int mipMapLevel = 0 ) const;
        virtual int getLineSize( unsigned int mipMapLevel = 0 ) const;

        protected:

        Texture2DFileInfo fileInfo;

        int bytesPerPixel;
        int width;
        int height;
        int size; // In bytes. Including padding at the end of each line (if present).
        bool mipmapsOnGpu;

        std::vector< std::vector<unsigned char> > dataMipMaps;

        Microsoft::WRL::ComPtr<ID3D11Texture2D>          texture;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResource;

        int getMaxMipMapCount();

        // Copying textures in not allowed.
        Texture2D( const Texture2D& ) = delete;
        Texture2D& operator=(const Texture2D&) = delete;
    };
}

