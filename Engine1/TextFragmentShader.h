#pragma once

#include "FragmentShader.h"

#include "Texture2D.h"

struct ID3D11SamplerState;

namespace Engine1
{
    class TextFragmentShader : public FragmentShader
    {
        public:
        TextFragmentShader();
        virtual ~TextFragmentShader();

        void compileFromFile( std::string path, ID3D11Device& device );
        void setParameters( ID3D11DeviceContext& deviceContext, ID3D11ShaderResourceView* characterTextureResource );

        private:

        Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;

        // Copying is not allowed.
        TextFragmentShader( const TextFragmentShader& ) = delete;
        TextFragmentShader& operator=(const TextFragmentShader&) = delete;
    };
}