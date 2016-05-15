#pragma once

#include "FragmentShader.h"

#include "TTexture2D.h"

#include "uchar4.h"
#include "float4.h"
#include "float2.h"

struct ID3D11SamplerState;

namespace Engine1
{
    class uchar4;

    class CombiningFragmentShader : public FragmentShader
    {
        public:

        CombiningFragmentShader();
        virtual ~CombiningFragmentShader();

        void compileFromFile( std::string path, ID3D11Device& device );
        void setParameters( ID3D11DeviceContext& deviceContext,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > srcTexture,
                            const float alpha );
        void unsetParameters( ID3D11DeviceContext& deviceContext );

        private:

        Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;

        // Copying is not allowed.
        CombiningFragmentShader( const CombiningFragmentShader& ) = delete;
        CombiningFragmentShader& operator=(const CombiningFragmentShader&) = delete;
    };
}

