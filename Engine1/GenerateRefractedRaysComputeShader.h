#include "ComputeShader.h"

#include <string>

#include "uchar4.h"
#include "float2.h"
#include "float3.h"
#include "float4.h"

#include "TTexture2D.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Engine1
{
    class GenerateRefractedRaysComputeShader : public ComputeShader
    {

        public:

        GenerateRefractedRaysComputeShader();
        virtual ~GenerateRefractedRaysComputeShader();

        void compileFromFile( std::string path, ID3D11Device& device );
        void setParameters( ID3D11DeviceContext& deviceContext, const unsigned int refractionLevel,
                            const Texture2DSpecBind< TexBind::ShaderResource, float4 >& rayDirectionTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, float4 >& rayHitPositionTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, float4 >& rayHitNormalTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& rayHitRoughnessTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& rayHitRefractiveIndexTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& contributionTermTexture,
                            const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > prevRefractiveIndexTexture,
                            const std::shared_ptr< const Texture2DSpecBind< TexBind::ShaderResource, unsigned char > > currentRefractiveIndexTexture );
        void unsetParameters( ID3D11DeviceContext& deviceContext );

        private:

        int m_resourceCount;

        __declspec(align(DIRECTX_CONSTANT_BUFFER_ALIGNMENT))
        struct ConstantBuffer
        {
            unsigned int refractionLevel;
            float3       pad1;
        };

        // Copying is not allowed.
        GenerateRefractedRaysComputeShader( const GenerateRefractedRaysComputeShader& ) = delete;
        GenerateRefractedRaysComputeShader& operator=(const GenerateRefractedRaysComputeShader&) = delete;
    };
}

