#include "TTexture2DBase.h"

#include <assert.h>

D3D11_USAGE Engine1::getTextureUsage( TTexture2DUsage usage )
{
    switch ( usage ) {
        case TTexture2DUsage::Immutable:
            return D3D11_USAGE_IMMUTABLE;
        case TTexture2DUsage::Dynamic:
            return D3D11_USAGE_DYNAMIC;
        case TTexture2DUsage::Default:
            return D3D11_USAGE_DEFAULT;
        case TTexture2DUsage::StagingRead:
        case TTexture2DUsage::StagingWrite:
        case TTexture2DUsage::StagingReadWrite:
            return D3D11_USAGE_STAGING;
    }

    assert( false );
    return D3D11_USAGE_DEFAULT; // To avoid warning.
}

UINT Engine1::getTextureCPUAccessFlags( TTexture2DUsage usage )
{
    switch ( usage ) {
        case TTexture2DUsage::Immutable:
            return 0;
        case TTexture2DUsage::Dynamic:
            return D3D11_CPU_ACCESS_WRITE;
        case TTexture2DUsage::Default:
            return 0;
        case TTexture2DUsage::StagingRead:
            return D3D11_CPU_ACCESS_READ;
        case TTexture2DUsage::StagingWrite:
            return D3D11_CPU_ACCESS_WRITE;
        case TTexture2DUsage::StagingReadWrite:
            return D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    }

    assert( false );
    return 0; // To avoid warning.
}

UINT Engine1::getTextureBindFlags( TTexture2DBinding binding )
{
    switch ( binding ) {
        case TTexture2DBinding::ShaderResource:
            return D3D11_BIND_SHADER_RESOURCE;
        case TTexture2DBinding::RenderTarget:
            return D3D11_BIND_RENDER_TARGET;
        case TTexture2DBinding::DepthStencil:
            return D3D11_BIND_DEPTH_STENCIL;
        case TTexture2DBinding::UnorderedAccess:
            return D3D11_BIND_UNORDERED_ACCESS;
        case TTexture2DBinding::RenderTarget_ShaderResource:
            return D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        case TTexture2DBinding::RenderTarget_UnorderedAccess:
            return D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;
        case TTexture2DBinding::RenderTarget_UnorderedAccess_ShaderResource:
            return D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
        case TTexture2DBinding::DepthStencil_ShaderResource:
            return D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        case TTexture2DBinding::DepthStencil_UnorderedAccess:
            return D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_UNORDERED_ACCESS;
        case TTexture2DBinding::DepthStencil_UnorderedAccess_ShaderResource:
            return D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
        case TTexture2DBinding::UnorderedAccess_ShaderResource:
            return D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
    }

    assert( false );
    return 0; // To avoid warning.
}

D3D11_MAP Engine1::getMapForWriteFlag( TTexture2DUsage usage )
{
    switch ( usage ) {
        case TTexture2DUsage::Immutable:
        case TTexture2DUsage::Default:
        case TTexture2DUsage::StagingRead:
            throw std::exception( "Engine1::getMapForWriteFlag - given usage doesn't support writing." );
        case TTexture2DUsage::Dynamic:
            return D3D11_MAP_WRITE_DISCARD;
        case TTexture2DUsage::StagingWrite:
            return D3D11_MAP_WRITE;
        case TTexture2DUsage::StagingReadWrite:
            return D3D11_MAP_READ_WRITE; // #TODO: can it be D3D11_MAP_WRITE? Would it have better performance?
    }

    assert( false );
    return D3D11_MAP_READ; // To avoid warning.
}

D3D11_MAP Engine1::getMapForReadFlag( TTexture2DUsage usage )
{
    switch ( usage ) {
        case TTexture2DUsage::Immutable:
        case TTexture2DUsage::Default:
        case TTexture2DUsage::Dynamic:
        case TTexture2DUsage::StagingWrite:
            throw std::exception( "Engine1::getMapForReadFlag - given usage doesn't support reading." );
        case TTexture2DUsage::StagingRead:
            return D3D11_MAP_READ;
        case TTexture2DUsage::StagingReadWrite:
            return D3D11_MAP_READ_WRITE; // #TODO: can it be D3D11_MAP_WRITE? Would it have better performance?
    }

    assert( false );
    return D3D11_MAP_READ; // To avoid warning.
}