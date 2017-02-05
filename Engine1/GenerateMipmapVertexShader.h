#pragma once

#include "VertexShader.h"

#include <string>

#include "float44.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class GenerateMipmapVertexShader : public VertexShader
    {

        public:

        GenerateMipmapVertexShader();
        virtual ~GenerateMipmapVertexShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device >& device );
        void setParameters( ID3D11DeviceContext& deviceContext );

        ID3D11InputLayout& getInputLauout() const;

        private:

        Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

        // Copying is not allowed.
        GenerateMipmapVertexShader( const GenerateMipmapVertexShader& ) = delete;
        GenerateMipmapVertexShader& operator=(const GenerateMipmapVertexShader&) = delete;
    };
}

