#pragma once

#include <string>
#include <vector>
#include <memory>

#include <d3d11_3.h>
#include <wrl.h>

#include "Texture2D.h"

// Everything can be treated as SRV and UAV, so we can use base Texture2D class as parameters,
// when no distinction is needed.

// Texture types:
// Immutable_UnorderedAccess_ShaderResource,
// Dynamic_UnorderedAccess_ShaderResource,
// Default_RenderTarget_UnorderedAccess_ShaderResource,
// Default_DepthStencil_ShaderResource

namespace Engine1
{
	template< typename PixelType >
	class ImmutableTexture2D : public Texture2D< PixelType >
	{
	public:

		ImmutableTexture2D() 
			: Texture2D(
				D3D11_USAGE_IMMUTABLE,
				D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE,
				0 )
		{}

		ImmutableTexture2D(
			ID3D11Device3& device,
			const Texture2DFileInfo& fileInfo, 
			const bool storeOnCpu, const bool storeOnGpu, 
			const bool generateMipmaps, 
			DXGI_FORMAT textureFormat, DXGI_FORMAT uavFormat, DXGI_FORMAT srtFormat)
			: Texture2D( 
				D3D11_USAGE_IMMUTABLE, 
				D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE, 
				0, 
				device, 
				fileInfo, 
				storeOnCpu, storeOnGpu,
				generateMipmaps, 
				textureFormat, uavFormat, srtFormat, DXGI_FORMAT_UNKNOWN )
		{}

		ImmutableTexture2D(
			ID3D11Device3& device, 
			std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
			const Texture2DFileInfo::Format format, 
			const bool storeOnCpu, const bool storeOnGpu, 
			const bool generateMipmaps,
			DXGI_FORMAT textureFormat, DXGI_FORMAT uavFormat, DXGI_FORMAT srtFormat) 
			: Texture2D(
				D3D11_USAGE_IMMUTABLE, 
				D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE, 
				0, 
				device, 
				dataIt, dataEndIt, 
				format, 
				storeOnCpu, storeOnGpu,
				generateMipmaps, 
				textureFormat, uavFormat, srtFormat, DXGI_FORMAT_UNKNOWN )
		{}

		ImmutableTexture2D(
			ID3D11Device3& device, 
			const int width, const int height, 
			const bool storeOnCpu, const bool storeOnGpu,
			const bool hasMipmaps, 
			DXGI_FORMAT textureFormat, DXGI_FORMAT uavFormat, DXGI_FORMAT srtFormat) 
			: Texture2D(
				D3D11_USAGE_IMMUTABLE, 
				D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE, 
				0, 
				device, 
				width, height, 
				storeOnCpu, storeOnGpu,
				hasMipmaps, 
				textureFormat, uavFormat, srtFormat, DXGI_FORMAT_UNKNOWN )
		{}

		ImmutableTexture2D(
			ID3D11Device3& device, 
			const std::vector< PixelType >& data, 
			const int width, const int height,
			const bool storeOnCpu, const bool storeOnGpu, 
			const bool generateMipmaps, 
			DXGI_FORMAT textureFormat, DXGI_FORMAT uavFormat, DXGI_FORMAT srtFormat) 
			
			: Texture2D(
				D3D11_USAGE_IMMUTABLE, 
				D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE, 
				0, 
				device, 
				data, 
				width, height, 
				storeOnCpu, storeOnGpu,
				generateMipmaps, 
				textureFormat, uavFormat, srtFormat, DXGI_FORMAT_UNKNOWN )
		{}
	};

	template< typename PixelType >
	class DynamicTexture2D : public Texture2D< PixelType >
    {
    public:

		DynamicTexture2D( 
			ID3D11Device3& device, 
			const Texture2DFileInfo& fileInfo, 
			const bool storeOnCpu, const bool storeOnGpu,
            const bool generateMipmaps, 
			DXGI_FORMAT textureFormat, DXGI_FORMAT uavFormat, DXGI_FORMAT srtFormat ) 
			: Texture2D( 
				D3D11_USAGE_DYNAMIC,
				D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE,
				D3D11_CPU_ACCESS_WRITE,
				device, 
				fileInfo, 
				storeOnCpu, storeOnGpu,
				generateMipmaps, 
				textureFormat, uavFormat, srtFormat, DXGI_FORMAT_UNKNOWN )
        {}

		DynamicTexture2D( 
			ID3D11Device3& device, 
			std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
            const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, const bool generateMipmaps,
            DXGI_FORMAT textureFormat, DXGI_FORMAT uavFormat, DXGI_FORMAT srtFormat ) 
			: Texture2D( 
				D3D11_USAGE_DYNAMIC,
				D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE,
				D3D11_CPU_ACCESS_WRITE, 
				device, 
				dataIt, dataEndIt, 
				format, 
				storeOnCpu, storeOnGpu,
				generateMipmaps, 
				textureFormat, uavFormat, srtFormat, DXGI_FORMAT_UNKNOWN )
        {}

		DynamicTexture2D( 
			ID3D11Device3& device, 
			const int width, const int height, 
			const bool storeOnCpu, const bool storeOnGpu,
            const bool hasMipmaps, 
			DXGI_FORMAT textureFormat, DXGI_FORMAT uavFormat, DXGI_FORMAT srtFormat ) 
			
			: Texture2D( 
				D3D11_USAGE_DYNAMIC,
				D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE,
				D3D11_CPU_ACCESS_WRITE, 
				device, 
				width, height, 
				storeOnCpu, storeOnGpu,
				hasMipmaps, 
				textureFormat, uavFormat, srtFormat, DXGI_FORMAT_UNKNOWN )
        {}

		DynamicTexture2D( 
			ID3D11Device3& device, 
			const std::vector< PixelType >& data, 
			const int width, const int height,
            const bool storeOnCpu, const bool storeOnGpu, 
			const bool generateMipmaps, 
			DXGI_FORMAT textureFormat, DXGI_FORMAT uavFormat, DXGI_FORMAT srtFormat ) 
			: Texture2D( 
				D3D11_USAGE_DYNAMIC,
				D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE,
				D3D11_CPU_ACCESS_WRITE, 
				device, 
				data, 
				width, height, 
				storeOnCpu, storeOnGpu,
				generateMipmaps, 
				textureFormat, uavFormat, srtFormat, DXGI_FORMAT_UNKNOWN )
        {}
    };

    template< typename PixelType >
	class DepthTexture2D : public Texture2D< PixelType >
    {
    public:

		DepthTexture2D( 
			ID3D11Device3& device, 
			const Texture2DFileInfo& fileInfo, 
			const bool storeOnCpu, const bool storeOnGpu,
            const bool generateMipmaps, 
			DXGI_FORMAT textureFormat, DXGI_FORMAT srtFormat, DXGI_FORMAT depthFormat ) 
			: Texture2D( 
				D3D11_USAGE_DEFAULT,
				D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE,
				0, 
				device, 
				fileInfo, 
				storeOnCpu, storeOnGpu,
				generateMipmaps, 
				textureFormat, DXGI_FORMAT_UNKNOWN, srtFormat, depthFormat )
        {}

		DepthTexture2D( 
			ID3D11Device3& device, 
			std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
            const Texture2DFileInfo::Format format, 
			const bool storeOnCpu, const bool storeOnGpu, 
			const bool generateMipmaps,
            DXGI_FORMAT textureFormat, DXGI_FORMAT srtFormat, DXGI_FORMAT depthFormat ) 
			: Texture2D( 
				D3D11_USAGE_DEFAULT,
				D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE,
				0, 
				device, 
				dataIt, dataEndIt, 
				format, 
				storeOnCpu, storeOnGpu,
				generateMipmaps, 
				textureFormat, DXGI_FORMAT_UNKNOWN, srtFormat, depthFormat )
        {}

		DepthTexture2D( 
			ID3D11Device3& device, 
			const int width, const int height, 
			const bool storeOnCpu, const bool storeOnGpu,
            const bool hasMipmaps, DXGI_FORMAT textureFormat, DXGI_FORMAT srtFormat, DXGI_FORMAT depthFormat ) 
			: Texture2D( 
				D3D11_USAGE_DEFAULT,
				D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE,
				0, 
				device, 
				width, height, 
				storeOnCpu, storeOnGpu,
				hasMipmaps, 
				textureFormat, DXGI_FORMAT_UNKNOWN, srtFormat, depthFormat )
        {}

		DepthTexture2D( 
			ID3D11Device3& device, 
			const std::vector< PixelType >& data, 
			const int width, const int height,
            const bool storeOnCpu, const bool storeOnGpu, 
			const bool generateMipmaps, DXGI_FORMAT textureFormat,
			DXGI_FORMAT srtFormat, DXGI_FORMAT depthFormat ) 
			: Texture2D( 
				D3D11_USAGE_DEFAULT,
				D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE,
				0, 
				device, 
				data, 
				width, height, 
				storeOnCpu, storeOnGpu,
				generateMipmaps, 
				textureFormat, DXGI_FORMAT_UNKNOWN, srtFormat, depthFormat )
        {}

		ID3D11DepthStencilView* getDepthStencilView( int mipmapLevel = 0 ) const
		{
			if (mipmapLevel < 0 || mipmapLevel >= (int)m_dsViews.size())
				throw std::exception("DepthTexture::getDepthStencilView - Tried to access depth stencil view for non-existing mipmap level.");

			return m_dsViews[mipmapLevel].Get();
		}

		void clearDepthStencilView(ID3D11DeviceContext3& deviceContext, bool clearDepth, float depth, bool clearStencil, unsigned char stencil, int mipmapLevel = 0)
		{
			if (!clearDepth && !clearStencil)
				return;

			if (!isInGpuMemory())
				throw std::exception("DepthTexture::clearDepthStencilView - Texture not in GPU memory.");

			if (mipmapLevel < 0 || mipmapLevel >= (int)m_dsViews.size())
				throw std::exception("DepthTexture::clearDepthStencilView - Tried to clear depth stencil view for non-existing mipmap level.");

			UINT flags = 0;
			if (clearDepth)   flags |= D3D11_CLEAR_DEPTH;
			if (clearStencil) flags |= D3D11_CLEAR_STENCIL;

			deviceContext.ClearDepthStencilView(m_dsViews[mipmapLevel].Get(), flags, depth, stencil);
		}
    };

	template< typename PixelType >
	class RenderTargetTexture2D : public Texture2D< PixelType >
    {
        public:

        RenderTargetTexture2D( 
			ID3D11Device3& device, const Texture2DFileInfo& fileInfo, 
			const bool storeOnCpu, const bool storeOnGpu,
            const bool generateMipmaps, 
			DXGI_FORMAT textureFormat, DXGI_FORMAT uavFormat, DXGI_FORMAT srtFormat, DXGI_FORMAT rtFormat ) 
			: Texture2D( 
				D3D11_USAGE_DEFAULT,
				D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE,
				0, 
				device, 
				fileInfo, 
				storeOnCpu, storeOnGpu,
				generateMipmaps, 
				textureFormat, uavFormat, srtFormat, rtFormat )
        {}

		RenderTargetTexture2D( 
			ID3D11Device3& device, 
			std::vector<char>::const_iterator dataIt, std::vector<char>::const_iterator dataEndIt,
			const Texture2DFileInfo::Format format, const bool storeOnCpu, const bool storeOnGpu, 
			const bool generateMipmaps,
			DXGI_FORMAT textureFormat, DXGI_FORMAT uavFormat, DXGI_FORMAT srtFormat, DXGI_FORMAT rtFormat ) 
			: Texture2D( 
				D3D11_USAGE_DEFAULT,
				D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE,
				0, 
				device, 
				dataIt, dataEndIt, 
				format, 
				storeOnCpu, storeOnGpu,
				generateMipmaps, 
				textureFormat, uavFormat, srtFormat, rtFormat )
        {}

		RenderTargetTexture2D( 
			ID3D11Device3& device, 
			const int width, const int height, 
			const bool storeOnCpu, const bool storeOnGpu,
            const bool hasMipmaps, 
			DXGI_FORMAT textureFormat, DXGI_FORMAT uavFormat, DXGI_FORMAT srtFormat, DXGI_FORMAT rtFormat ) 
			: Texture2D( 
				D3D11_USAGE_DEFAULT,
				D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE,
				0, 
				device, 
				width, height, 
				storeOnCpu, storeOnGpu,
				hasMipmaps, 
				textureFormat, uavFormat, srtFormat, rtFormat )
        {}

		RenderTargetTexture2D( 
			ID3D11Device3& device, 
			const std::vector< PixelType >& data, 
			const int width, const int height,
			const bool storeOnCpu, const bool storeOnGpu, 
			const bool generateMipmaps, 
			DXGI_FORMAT textureFormat, DXGI_FORMAT uavFormat, DXGI_FORMAT srtFormat, DXGI_FORMAT rtFormat ) 
			: Texture2D( 
				D3D11_USAGE_DEFAULT,
				D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE,
				0, 
				device, 
				data, 
				width, height, 
				storeOnCpu, storeOnGpu,
				generateMipmaps, 
				textureFormat, uavFormat, srtFormat, rtFormat )
        {}

		// Specific to render target - useful to create a texture from the frame back-buffer.
		RenderTargetTexture2D(
			ID3D11Device3& device, 
			Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture ) 
			: Texture2D( 
				D3D11_USAGE_DEFAULT,
				D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE,
				0, 
				device, 
				texture )
		{}

		ID3D11RenderTargetView* getRenderTargetView(int mipmapLevel = 0) const
		{
			if (mipmapLevel < 0 || mipmapLevel >= (int)m_rtViews.size())
				throw std::exception("DepthTexture::getRenderTargetView - Tried to access render target view for non-existing mipmap level.");

			return m_rtViews[mipmapLevel].Get();
		}

		void clearRenderTargetView(ID3D11DeviceContext3& deviceContext, float4 colorRGBA, int mipmapLevel = 0)
		{
			if (!isInGpuMemory())
				throw std::exception("DepthTexture::clearRenderTargetView - Texture not in GPU memory.");

			if (mipmapLevel < 0 || mipmapLevel >= (int)m_rtViews.size())
				throw std::exception("DepthTexture::clearRenderTargetView - Tried to clear render target view for non-existing mipmap level.");

			deviceContext.ClearRenderTargetView(m_rtViews[mipmapLevel].Get(), colorRGBA.getData());
		}
    };
}
