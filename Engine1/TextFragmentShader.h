#pragma once

#include "FragmentShader.h"

struct ID3D11DeviceContext3;
struct ID3D11SamplerState;
struct ID3D11ShaderResourceView;

namespace Engine1
{
    class TextFragmentShader : public FragmentShader
    {
        public:
        TextFragmentShader();
        virtual ~TextFragmentShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );
        void setParameters( ID3D11DeviceContext3& deviceContext, ID3D11ShaderResourceView* characterTextureResource );

        private:

        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;

        // Copying is not allowed.
        TextFragmentShader( const TextFragmentShader& ) = delete;
        TextFragmentShader& operator=(const TextFragmentShader&) = delete;
    };
}