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

    class RefractionShadingComputeShader : public ComputeShader
    {

        public:

        RefractionShadingComputeShader();
        virtual ~RefractionShadingComputeShader();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        void setParameters( ID3D11DeviceContext3& deviceContext, const float3& cameraPos,
                            const std::shared_ptr< Texture2D< float4 > > positionTexture,
                            const std::shared_ptr< Texture2D< float4 > > normalTexture,
                            /*const std::shared_ptr< Texture2D< uchar4 > > depthTexture,*/
                            /*const std::shared_ptr< Texture2D< float > >  hitDistanceTexture,*/
                            const std::shared_ptr< Texture2D< uchar4 > > albedoTexture,
                            const std::shared_ptr< Texture2D< unsigned char > > metalnessTexture,
                            const std::shared_ptr< Texture2D< unsigned char > > roughnessTexture,
                            const std::shared_ptr< Texture2D< uchar4 > > previousContributionTermRoughnessTexture );

        void unsetParameters( ID3D11DeviceContext3& deviceContext );

        private:

        __declspec(align(DIRECTX_CONSTANT_BUFFER_ALIGNMENT))
        struct ConstantBuffer
        {
            float3 cameraPos;
            float  pad1;
        };

        // Copying is not allowed.
        RefractionShadingComputeShader( const RefractionShadingComputeShader& )          = delete;
        RefractionShadingComputeShader& operator=(const RefractionShadingComputeShader&) = delete;
    };
}

