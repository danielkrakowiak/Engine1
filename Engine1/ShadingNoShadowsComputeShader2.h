#pragma once

#include "ComputeShader.h"

#include <string>

#include "uchar4.h"
#include "float2.h"
#include "float3.h"
#include "float4.h"
#include "float44.h"

#include "Texture2D.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;

namespace Engine1
{
    class Light;

    class ShadingNoShadowsComputeShader2 : public ComputeShader
    {

        public:

        ShadingNoShadowsComputeShader2();
        virtual ~ShadingNoShadowsComputeShader2();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        void setParameters( ID3D11DeviceContext3& deviceContext,
                            const std::shared_ptr< Texture2D< float4 > > rayOriginTexture,
                            const std::shared_ptr< Texture2D< float4 > > rayHitPositionTexture,
                            const std::shared_ptr< Texture2D< uchar4 > > rayHitAlbedoTexture,
                            const std::shared_ptr< Texture2D< unsigned char > > rayHitMetalnessTexture,
                            const std::shared_ptr< Texture2D< unsigned char > > rayHitRoughnessTexture,
                            const std::shared_ptr< Texture2D< float4 > > rayHitNormalTexture,
                            const std::vector< std::shared_ptr< Light > >& lights );

        void unsetParameters( ID3D11DeviceContext3& deviceContext );

        private:

        static const unsigned int maxPointLightCount = 50;

        __declspec( align( DIRECTX_CONSTANT_BUFFER_ALIGNMENT ) )
        struct ConstantBuffer
        {
            unsigned int pointLightCount;
            float3       pad1;
            float4       pointLightPositions[ maxPointLightCount ];
            float4       pointLightColors[ maxPointLightCount ];
            float4       lightLinearAttenuationFactor[ maxPointLightCount ];   
            float4       lightQuadraticAttenuationFactor[ maxPointLightCount ];
        };

        // Copying is not allowed.
        ShadingNoShadowsComputeShader2( const ShadingNoShadowsComputeShader2& ) = delete;
        ShadingNoShadowsComputeShader2& operator=( const ShadingNoShadowsComputeShader2& ) = delete;
    };
}
