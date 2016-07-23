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
    class GenerateReflectedRaysComputeShader : public ComputeShader
    {

        public:

        GenerateReflectedRaysComputeShader();
        virtual ~GenerateReflectedRaysComputeShader();

        void compileFromFile( std::string path, ID3D11Device& device );
        void setParameters( ID3D11DeviceContext& deviceContext,
                            const Texture2DSpecBind< TexBind::ShaderResource, float4 >& rayDirectionTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, float4 >& rayHitPositionTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, float4 >& rayHitNormalTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, unsigned char >& rayHitRoughnessTexture,
                            const Texture2DSpecBind< TexBind::ShaderResource, uchar4 >& reflectionTermTexture );
        void unsetParameters( ID3D11DeviceContext& deviceContext );

        private:

        // Copying is not allowed.
        GenerateReflectedRaysComputeShader( const GenerateReflectedRaysComputeShader& ) = delete;
        GenerateReflectedRaysComputeShader& operator=(const GenerateReflectedRaysComputeShader&) = delete;
    };
}

