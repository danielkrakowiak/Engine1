#include "ComputeShader.h"

#include <string>

#include "uchar4.h"
#include "float2.h"
#include "float3.h"
#include "float4.h"

#include "Texture2D.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;

namespace Engine1
{
    class GenerateFirstRefractedRaysComputeShader : public ComputeShader
    {

        public:

        GenerateFirstRefractedRaysComputeShader();
        virtual ~GenerateFirstRefractedRaysComputeShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );
        void setParameters( ID3D11DeviceContext3& deviceContext, const float3 cameraPos, const float3 viewportCenter, 
                            const float3 viewportUp, const float3 viewportRight, const float2 viewportSize,
                            const Texture2D< float4 >& positionTexture,
                            const Texture2D< float4 >& normalTexture,
                            const Texture2D< unsigned char >& roughnessTexture,
                            const Texture2D< unsigned char >& refractiveIndexTexture,
                            const Texture2D< uchar4 >& contributionTermTexture,
                            const int outputTextureWidth, const int outputTextureHeight );
        void unsetParameters( ID3D11DeviceContext3& deviceContext );

        private:

        int m_resourceCount;

        __declspec(align(DIRECTX_CONSTANT_BUFFER_ALIGNMENT))
        struct ConstantBuffer
        {
            float3 cameraPos;
            float  pad1;
            float3 viewportCenter;
            float  pad2;
            float3 viewportUp;
            float  pad3;
            float3 viewportRight;
            float  pad4;
            float2 viewportSizeHalf;
            float2 pad5;
            float2 outputTextureSize;
            float2 pad6;
        };

        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerStateLinearFilter;

        // Copying is not allowed.
        GenerateFirstRefractedRaysComputeShader( const GenerateFirstRefractedRaysComputeShader& ) = delete;
        GenerateFirstRefractedRaysComputeShader& operator=(const GenerateFirstRefractedRaysComputeShader&) = delete;
    };
}

