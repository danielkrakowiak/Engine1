#pragma once

#include "FragmentShader.h"

#include "Texture2D.h"

#include "uchar4.h"
#include "float4.h"
#include "float2.h"

struct ID3D11SamplerState;

namespace Engine1
{
    class uchar4;

    class TextureFragmentShader : public FragmentShader
    {
        public:

        TextureFragmentShader();
        virtual ~TextureFragmentShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );
        void setParameters( ID3D11DeviceContext3& deviceContext, const Texture2D< unsigned char >& texture, int mipmapLevel = 0 );
        void setParameters( ID3D11DeviceContext3& deviceContext, const Texture2D< uchar4 >& texture, int mipmapLevel = 0 );
        void setParameters( ID3D11DeviceContext3& deviceContext, const Texture2D< float4 >& texture, int mipmapLevel = 0 );
        void setParameters( ID3D11DeviceContext3& deviceContext, const Texture2D< float2 >& texture, int mipmapLevel = 0 );
        void setParameters( ID3D11DeviceContext3& deviceContext, const Texture2D< float  >& texture, int mipmapLevel = 0 );
        void unsetParameters( ID3D11DeviceContext3& deviceContext );

        private:

        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;

        // Copying is not allowed.
        TextureFragmentShader( const TextureFragmentShader& ) = delete;
        TextureFragmentShader& operator=(const TextureFragmentShader&) = delete;
    };
}

