#pragma once

#include "VertexShader.h"

#include <string>

#include "float44.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;

namespace Engine1
{
    class BlockMeshVertexShader : public VertexShader
    {

        public:

        BlockMeshVertexShader();
        virtual ~BlockMeshVertexShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );
        void setParameters( ID3D11DeviceContext3& deviceContext, const float43& worldMatrix, const float44& viewMatrix, const float44& projectionMatrix );

        ID3D11InputLayout& getInputLauout() const;

        private:

        Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

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

