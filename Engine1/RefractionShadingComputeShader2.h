#pragma once

#include "ComputeShader.h"

#include <string>

#include "uchar4.h"
#include "float2.h"
#include "float3.h"
#include "float4.h"
#include "float44.h"

#include "Texture2D.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class Light;

    class RefractionShadingComputeShader2 : public ComputeShader
    {

        public:

        RefractionShadingComputeShader2();
        virtual ~RefractionShadingComputeShader2();

        void initialize( Microsoft::WRL::ComPtr< ID3D11Device >& device );

        void setParameters( ID3D11DeviceContext& deviceContext,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > rayOriginTexture,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > positionTexture,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
                            /*const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > depthTexture,*/
                            /*const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float > >  hitDistanceTexture,*/
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > albedoTexture,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > metalnessTexture,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > roughnessTexture,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > previousContributionTermRoughnessTexture );

        void unsetParameters( ID3D11DeviceContext& deviceContext );

        private:

        // Copying is not allowed.
        RefractionShadingComputeShader2( const RefractionShadingComputeShader2& )          = delete;
        RefractionShadingComputeShader2& operator=(const RefractionShadingComputeShader2&) = delete;
    };
}

