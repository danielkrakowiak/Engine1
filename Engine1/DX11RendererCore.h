#pragma once

#include <vector>
#include <memory>

#include "Texture2DTypes.h"
#include "StagingTexture2D.h"

#include "uint3.h"
#include "float2.h"
#include "float4.h"
#include "uchar4.h"

struct ID3D11DeviceContext3;
struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11BlendState;
struct ID3D11Buffer;
struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;

namespace Engine1
{
    class VertexShader;
    class FragmentShader;
    class ComputeShader;
    class RectangleMesh;
    class BlockMesh;
    class SkeletonMesh;
    class FontCharacter;

	struct RenderTargets
	{
		std::vector< std::shared_ptr< RenderTargetTexture2D< float > > >         typeFloat;
		std::vector< std::shared_ptr< RenderTargetTexture2D< float2 > > >        typeFloat2;
		std::vector< std::shared_ptr< RenderTargetTexture2D< float3 > > >        typeFloat3;
		std::vector< std::shared_ptr< RenderTargetTexture2D< float4 > > >        typeFloat4;
		std::vector< std::shared_ptr< RenderTargetTexture2D< unsigned char > > > typeUchar;
		std::vector< std::shared_ptr< RenderTargetTexture2D< uchar4 > > >        typeUchar4;
		std::shared_ptr< DepthTexture2D< uchar4 > >                              depthStencil;
		std::shared_ptr< DepthTexture2D< float > >                               depth;

		ID3D11RenderTargetView*    getRTV( size_t idx, int mipmapLevel = 0 ) const;
		ID3D11UnorderedAccessView* getUAV( size_t idx, int mipmapLevel = 0 ) const;

		size_t getCount() const;
	};

    class DX11RendererCore
    {

        public:

        DX11RendererCore();
        ~DX11RendererCore();

        void initialize( ID3D11DeviceContext3& deviceContext );

        void disableRenderingPipeline();
        void disableComputePipeline();

        void setViewport( float2 dimensions, float2 topLeft = float2::ZERO, float depthMin = 0.0f, float depthMax = 1.0f );

        void enableRenderTargets( 
            const RenderTargets& renderTargets,
			const RenderTargets& unorderedAccessTargets,
            const int mipmapLevel = 0 
        );

        void disableRenderTargets();

        // #TODO: Remove version with shared_ptr.
        void enableRenderingShaders( std::shared_ptr<const VertexShader> vertexShader, std::shared_ptr<const FragmentShader> fragmentShader );

        void enableRenderingShaders( const VertexShader& vertexShader );
        void enableRenderingShaders( const VertexShader& vertexShader, const FragmentShader& fragmentShader );

        // #TODO: Remove version with shared_ptr.
        void enableComputeShader( std::shared_ptr<const ComputeShader> computeShader );

        void enableComputeShader( const ComputeShader& computeShader );

        void disableRenderingShaders();
        void disableComputeShaders();

        void enableRasterizerState( ID3D11RasterizerState& rasterizerState );
        void enableDepthStencilState( ID3D11DepthStencilState& depthStencilState );
        void enableBlendState( ID3D11BlendState& blendState );
        void enableDefaultRasterizerState();
        void enableDefaultDepthStencilState();
        void enableDefaultBlendState();

        void draw( const RectangleMesh& mesh );
        void draw( const BlockMesh& mesh );
        void draw( const SkeletonMesh& mesh );
        void draw( const FontCharacter& character );

        void compute( uint3 groupCount );

        void disableShaderInputs();

        template< typename T >
        void copyTextureGpu( Texture2D< T >& destTexture, const int destMipmap,
                          const Texture2D< T >& srcTexture, const int srcMipmap );

		template< typename T >
		void copyTextureGpu( Texture2D< T >& destTexture,
					      const Texture2D< T >& srcTexture,
						  const int2 coords = int2( 0, 0 ), int2 dimensions = int2( -1, -1 ) );

        template< typename T >
        void copyTextureGpu( StagingTexture2D< T >& destTexture, const Texture2D< T >& srcTexture );

        template< typename T >
        void copyTextureGpu(
            StagingTexture2D< T >& destTexture, unsigned int destMipmap,
            const Texture2D< T >& srcTexture, unsigned int srcMipmap);

        template< typename T >
        void copyTextureGpu( StagingTexture2D< T >& destTexture, const Texture2D< T >& srcTexture,
                          const int2 coords, const int2 dimensions );

        private:

        ID3D11DeviceContext* m_deviceContext;

        bool m_graphicsShaderEnabled;
        bool m_computeShaderEnabled;

        // Used only for comparison.
        const VertexShader*   m_currentVertexShader;
        const FragmentShader* m_currentFragmentShader;
        const ComputeShader*  m_currentComputeShader;

        float2 viewportDimensions;
        float2 viewportTopLeft;
        float  viewportDepthMin;
        float  viewportDepthMax;

        std::vector< ID3D11RenderTargetView* >    m_currentRTVs;
        ID3D11DepthStencilView*                   m_currentDSV;
        std::vector< ID3D11UnorderedAccessView* > m_currentUAVs;

        ID3D11RasterizerState*   m_currentRasterizerState;
        ID3D11DepthStencilState* m_currentDepthStencilState;
        ID3D11BlendState*        m_currentBlendState;

        void createNullShaderInputs();

        std::vector<ID3D11Buffer*>             m_nullVertexBuffers;
        std::vector<unsigned int>              m_nullVertexBuffersStrideOffset;
        std::vector<ID3D11ShaderResourceView*> m_nullResources;
        std::vector<ID3D11SamplerState*>       m_nullSamplers;

        // Copying is not allowed.
        DX11RendererCore( const DX11RendererCore& ) = delete;
        DX11RendererCore& operator=(const DX11RendererCore&) = delete;
    };

	template< typename T >
	void DX11RendererCore::copyTextureGpu( Texture2D< T >& destTexture,
											const Texture2D< T >& srcTexture,
											const int2 coords, int2 dimensions )
	{
		if ( !m_deviceContext ) {
            throw std::exception( "Direct3DRendererCore::copyTexture - renderer not initialized." );
        }

        if ( dimensions.x == -1 && dimensions.y == -1 ) {
            dimensions = srcTexture.getDimensions();
        }

		if ( coords.x < 0 ||
             coords.y < 0 ||
             dimensions.x < 0 ||
             dimensions.y < 0 ||
             coords.x + dimensions.x > srcTexture.getWidth() ||
             coords.y + dimensions.y > srcTexture.getHeight() )
        {
			throw std::exception( "Direct3DRendererCore::copyTexture - given fragment exceeds boundaries of the source/destination texture." );
        }

		D3D11_BOX sourceRregion;
		sourceRregion.left   = coords.x;
		sourceRregion.right  = coords.x + dimensions.x;
		sourceRregion.top    = coords.y;
		sourceRregion.bottom = coords.y + dimensions.y;
		sourceRregion.front  = 0;
		sourceRregion.back   = 1;

		m_deviceContext->CopySubresourceRegion( 
            destTexture.getTextureResource().Get(), 0, 
            coords.x, coords.y, 0, 
            srcTexture.getTextureResource().Get(), 0,
            &sourceRregion 
        );
	}

    template< typename T >
    void DX11RendererCore::copyTextureGpu( Texture2D< T >& destTexture, const int destMipmap,
                                            const Texture2D< T >& srcTexture, const int srcMipmap )
    {
        if ( !m_deviceContext ) {
            throw std::exception( "Direct3DRendererCore::copyTexture - renderer not initialized." );
        }

        D3D11_BOX sourceRregion;
        sourceRregion.left   = 0;
        sourceRregion.right  = srcTexture.getWidth();
        sourceRregion.top    = 0;
        sourceRregion.bottom = srcTexture.getHeight();
        sourceRregion.front  = 0;
        sourceRregion.back   = 1;

        m_deviceContext->CopySubresourceRegion( 
            destTexture.getTextureResource().Get(), 
            (UINT)destMipmap, 0, 0, 0, 
            srcTexture.getTextureResource().Get(), 
            srcMipmap, 
            &sourceRregion 
        );
    }

    template< typename T >
    void DX11RendererCore::copyTextureGpu( StagingTexture2D< T >& destTexture, const Texture2D< T >& srcTexture )
    {
        if ( !m_deviceContext ) {
            throw std::exception( "Direct3DRendererCore::copyTexture - renderer not initialized." );
        }

        m_deviceContext->CopyResource( destTexture.getTextureResource().Get(), srcTexture.getTextureResource().Get() );
    }

    template< typename T >
    void DX11RendererCore::copyTextureGpu(
        StagingTexture2D< T >& destTexture, unsigned int destMipmap,
        const Texture2D< T >& srcTexture, unsigned int srcMipmap)
    {
        if (!m_deviceContext) {
            throw std::exception("Direct3DRendererCore::copyTexture - renderer not initialized.");
        }

        m_deviceContext->CopySubresourceRegion(
            destTexture.getTextureResource().Get(), destMipmap, 0u, 0u, 0u,
            srcTexture.getTextureResource().Get(), srcMipmap, nullptr);
    }

    template< typename T >
    void DX11RendererCore::copyTextureGpu( StagingTexture2D< T >& destTexture, const Texture2D< T >& srcTexture,
                                            const int2 coords, const int2 dimensions )
    {
        if ( !m_deviceContext ) {
            throw std::exception( "Direct3DRendererCore::copyTexture - renderer not initialized." );
        }

        if ( coords.x < 0 || 
             coords.y < 0 || 
             dimensions.x < 0 || 
             dimensions.y < 0 || 
             coords.x + dimensions.x > srcTexture.getWidth() || 
             coords.y + dimensions.y > srcTexture.getHeight() )
        {
            throw std::exception( "Direct3DRendererCore::copyTexture - given fragment exceeds boundaries of the source/destination texture." );
        }

        D3D11_BOX sourceRregion;
        sourceRregion.left   = coords.x;
        sourceRregion.right  = coords.x + dimensions.x;
        sourceRregion.top    = coords.y;
        sourceRregion.bottom = coords.y + dimensions.y;
        sourceRregion.front  = 0;
        sourceRregion.back   = 1;

        m_deviceContext->CopySubresourceRegion( 
            destTexture.getTextureResource().Get(), 0, 
            coords.x, coords.y, 0, 
            srcTexture.getTextureResource().Get(), 0, 
            &sourceRregion 
        );
    }
}

