#pragma once

#include <vector>
#include <memory>

#include "Texture2D.h"
#include "float4.h"
#include "int2.h"

namespace Engine1
{
    class RenderTargetManager
    {
        public:

        RenderTargetManager( ID3D11Device& device, int2 imageDimensions );
        ~RenderTargetManager();

        template < typename T >
        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, T > > getRenderTarget();

        private:

        std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float > > >         m_renderTargetsFloat;
        std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, float4 > > >        m_renderTargetsFloat4;
        std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, unsigned char > > > m_renderTargetsUchar;
        std::vector< std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget_UnorderedAccess_ShaderResource, uchar4 > > >        m_renderTargetsUchar4;
    };
}

