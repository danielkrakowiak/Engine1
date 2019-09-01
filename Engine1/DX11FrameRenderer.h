#pragma once

#include <string>
#include <utility>
#include <tuple>
#include <memory>
#include <Windows.h>
#include <wrl.h>

#include "RectangleMesh.h"

#include "TextureVertexShader.h"
#include "TextureFragmentShader.h"
#include "TextVertexShader.h"
#include "TextFragmentShader.h"
#include "Texture2DTypes.h"

#include "float44.h"
#include "uchar4.h"

#include "Font.h"

struct ID3D11Device33;
struct ID3D11DeviceContext3;
struct IDXGIAdapter;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
struct ID3D11RasterizerState;
struct ID3D11BlendState;

namespace Engine1
{
    class DX11RendererCore;
    class RenderTarget2D;
    class Profiler;

    class DX11FrameRenderer
    {

        public:

        DX11FrameRenderer( DX11RendererCore& rendererCore, Profiler& profiler );
        ~DX11FrameRenderer();

        void initialize( HWND windowHandle, int screenWidth, int screenHeight, bool fullscreen, bool verticalSync );

        void reportLiveObjects();

        void renderTexture( const Texture2D< unsigned char >&  texture, float posX, float posY, float width, float height, bool blend, int mipmapLevel = 0 );
        void renderTexture( const Texture2D< uchar4 >&         texture, float posX, float posY, float width, float height, bool blend, int mipmapLevel = 0 );
        void renderTexture( const Texture2D< float4 >&         texture, float posX, float posY, float width, float height, bool blend, int mipmapLevel = 0 );
        void renderTexture( const Texture2D< float2  >&        texture, float posX, float posY, float width, float height, bool blend, int mipmapLevel = 0 );
        void renderTexture( const Texture2D< float  >&         texture, float posX, float posY, float width, float height, bool blend, int mipmapLevel = 0 );

        void renderTextureAlpha( const Texture2D< uchar4 >& texture, float posX, float posY, float width, float height, bool blend, int mipmapLevel = 0 );

        void displayFrame();

        Microsoft::WRL::ComPtr< ID3D11Device3 > getDevice();
        Microsoft::WRL::ComPtr< ID3D11DeviceContext3 > getDeviceContext();

        std::string getGPUName() const;

        private:

        DX11RendererCore& m_rendererCore;
        Profiler&             m_profiler;

        // Initalization.
        std::tuple<int, int>    getRefreshRateNumeratorDenominator( IDXGIAdapter& adapter, unsigned int screenWidth, unsigned int screenHeight );
        std::string             getGpuDescription( IDXGIAdapter& adapter );
        size_t                  getGpuMemory( IDXGIAdapter& adapter );

        Microsoft::WRL::ComPtr<IDXGISwapChain> createSwapChain( 
            IDXGIFactory3& factory, ID3D11Device3& device,
            HWND windowHandle, bool fullscreen, bool verticalSync, 
            unsigned int screenWidth, unsigned int screenHeight, 
            int refreshRateNumerator, int refreshRateDenominator 
        );

        Microsoft::WRL::ComPtr<ID3D11Texture2D>       getBackbufferTexture( IDXGISwapChain& swapChain );
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> createRasterizerState( ID3D11Device3& device );
        Microsoft::WRL::ComPtr<ID3D11BlendState>      createBlendStateNoBlending( ID3D11Device3& device );
        Microsoft::WRL::ComPtr<ID3D11BlendState>      createBlendStateWithBlending( ID3D11Device3& device );

        bool m_initialized;

        bool m_fullscreen;
        int m_screenWidth, m_screenHeight;
        bool m_verticalSync;

        size_t m_gpuMemory;
        std::string m_gpuDescription;

        Microsoft::WRL::ComPtr< ID3D11Device3 >         m_device;
        Microsoft::WRL::ComPtr< ID3D11DeviceContext3 >  m_deviceContext;
        Microsoft::WRL::ComPtr< IDXGISwapChain >        m_swapChain;
        Microsoft::WRL::ComPtr< ID3D11RasterizerState > m_rasterizerState;
        Microsoft::WRL::ComPtr< ID3D11BlendState >      m_blendStateNoBlending;
        Microsoft::WRL::ComPtr< ID3D11BlendState >      m_blendStateWithBlending;

        std::shared_ptr< RenderTargetTexture2D< uchar4 > > m_renderTarget;

        // Default mesh.
        RectangleMesh rectangleMesh;

        // Shaders.
        void loadAndCompileShaders( Microsoft::WRL::ComPtr< ID3D11Device3 >& device );

        std::shared_ptr<TextureVertexShader>   m_textureVertexShader;
        std::shared_ptr<TextureFragmentShader> m_textureFragmentShader;
        std::shared_ptr<TextureFragmentShader> m_textureAlphaFragmentShader;
        std::shared_ptr<TextureFragmentShader> m_textureSingleChannelFragmentShader;
        std::shared_ptr<TextVertexShader>      m_textVertexShader;
        std::shared_ptr<TextFragmentShader>    m_textFragmentShader;

        // Copying is not allowed.
        DX11FrameRenderer( const DX11FrameRenderer& ) = delete;
        DX11FrameRenderer& operator=(const DX11FrameRenderer&) = delete;
    };
}

