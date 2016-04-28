#pragma once

namespace Engine1
{
    enum class TexUsage : int
    {
        Immutable = 0,
        Dynamic,
        Default,
    };

    enum class TexBind : int
    {
        ShaderResource = 0,
        RenderTarget,
        DepthStencil,
        UnorderedAccess,
        RenderTarget_ShaderResource,
        RenderTarget_UnorderedAccess,
        RenderTarget_UnorderedAccess_ShaderResource,
        DepthStencil_ShaderResource,
        UnorderedAccess_ShaderResource
    };
}