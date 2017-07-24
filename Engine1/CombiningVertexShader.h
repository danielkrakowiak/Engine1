#pragma once

#include "VertexShader.h"

#include <string>

#include "float44.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;

namespace Engine1
{
    class CombiningVertexShader : public VertexShader
    {

        public:

        CombiningVertexShader();
        virtual ~CombiningVertexShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );
        void setParameters( ID3D11DeviceContext3& deviceContext );

        ID3D11InputLayout& getInputLauout() const;

        private:

        Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

        /*__declspec(align(DIRECTX_CONSTANT_BUFFER_ALIGNMENT))
        struct ConstantBuffer
        {
            float  alpha;
            float3 pad1;
        };*/

        // Copying is not allowed.
        CombiningVertexShader( const CombiningVertexShader& ) = delete;
        CombiningVertexShader& operator=(const CombiningVertexShader&) = delete;
    };
}

