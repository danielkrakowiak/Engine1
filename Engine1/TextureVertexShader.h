#pragma once

#include "VertexShader.h"

#include <string>

#include "float44.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class TextureVertexShader : public VertexShader
    {

        public:

        TextureVertexShader();
        virtual ~TextureVertexShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device >& device );
        void setParameters( ID3D11DeviceContext& deviceContext, float posX, float posY, float width, float height );

        ID3D11InputLayout& getInputLauout() const;

        private:

        Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

        __declspec(align(DIRECTX_CONSTANT_BUFFER_ALIGNMENT))
        struct ConstantBuffer
        {
            float posX;
            float posY;
            float width;
            float height;
        };

        // Copying is not allowed.
        TextureVertexShader( const TextureVertexShader& ) = delete;
        TextureVertexShader& operator=(const TextureVertexShader&) = delete;
    };
}

