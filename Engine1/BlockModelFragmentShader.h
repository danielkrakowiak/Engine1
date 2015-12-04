#pragma once

#include "FragmentShader.h"

#include "Texture2D.h"

struct ID3D11SamplerState;

namespace Engine1
{
    class BlockModelFragmentShader : public FragmentShader
    {
        public:
        BlockModelFragmentShader();
        virtual ~BlockModelFragmentShader();

        void compileFromFile( std::string path, ID3D11Device& device );
        void setParameters( ID3D11DeviceContext& deviceContext, const Texture2D& albedoTexture );
        void unsetParameters( ID3D11DeviceContext& deviceContext );

        private:

        Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;

        // Copying is not allowed.
        BlockModelFragmentShader( const BlockModelFragmentShader& ) = delete;
        BlockModelFragmentShader& operator=(const BlockModelFragmentShader&) = delete;
    };
}