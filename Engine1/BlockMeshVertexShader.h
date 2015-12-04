#pragma once

#include "VertexShader.h"

#include <string>

#include "float44.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class BlockMeshVertexShader : public VertexShader
    {

        public:

        BlockMeshVertexShader();
        virtual ~BlockMeshVertexShader();

        void compileFromFile( std::string path, ID3D11Device& device );
        void setParameters( ID3D11DeviceContext& deviceContext, const float43& worldMatrix, const float44& viewMatrix, const float44& projectionMatrix );

        ID3D11InputLayout& getInputLauout() const;

        private:

        Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

        __declspec(align(DIRECTX_CONSTANT_BUFFER_ALIGNMENT))
        struct ConstantBuffer
        {
            float44 world;
            float44 view;
            float44 projection;
        };

        // Copying is not allowed.
        BlockMeshVertexShader( const BlockMeshVertexShader& ) = delete;
        BlockMeshVertexShader& operator=(const BlockMeshVertexShader&) = delete;
    };
}

