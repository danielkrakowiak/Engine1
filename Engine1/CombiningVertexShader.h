#pragma once

#include "VertexShader.h"

#include <string>

#include "float44.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class CombiningVertexShader : public VertexShader
    {

        public:

        CombiningVertexShader();
        virtual ~CombiningVertexShader();

        void compileFromFile( std::string path, ID3D11Device& device );
        void setParameters( ID3D11DeviceContext& deviceContext );

        ID3D11InputLayout& getInputLauout() const;

        private:

        Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

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

