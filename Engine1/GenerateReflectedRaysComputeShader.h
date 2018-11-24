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
    class GenerateReflectedRaysComputeShader : public ComputeShader
    {

        public:

        GenerateReflectedRaysComputeShader();
        virtual ~GenerateReflectedRaysComputeShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );
        void setParameters( ID3D11DeviceContext3& deviceContext,
                            const Texture2D< float4 >& rayDirectionTexture,
                            const Texture2D< float4 >& rayHitPositionTexture,
                            const Texture2D< float4 >& rayHitNormalTexture,
                            const Texture2D< unsigned char >& rayHitRoughnessTexture,
                            const Texture2D< uchar4 >& contributionTermTexture,
                            const int outputTextureWidth, const int outputTextureHeight );
        void unsetParameters( ID3D11DeviceContext3& deviceContext );

        private:

        __declspec(align(DIRECTX_CONSTANT_BUFFER_ALIGNMENT))
        struct ConstantBuffer
        {
            float2 outputTextureSize;
            float2 pad1;
        };

        Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerStateLinearFilter;

        // Copying is not allowed.
        GenerateReflectedRaysComputeShader( const GenerateReflectedRaysComputeShader& ) = delete;
        GenerateReflectedRaysComputeShader& operator=(const GenerateReflectedRaysComputeShader&) = delete;
    };
}

