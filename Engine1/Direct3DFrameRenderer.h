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

#include "float44.h"
#include "uchar4.h"

#include "Font.h"

struct ID3D11Device3;
struct ID3D11DeviceContext3;
struct IDXGIAdapter;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
struct ID3D11RasterizerState;
struct ID3D11BlendState;

namespace Engine1
{
    class Direct3DRendererCore;
    class RenderTarget2D;

    class Direct3DFrameRenderer
    {

        public:

        Direct3DFrameRenderer( Direct3DRendererCore& rendererCore );
        ~Direct3DFrameRenderer();

        void initialize( HWND windowHandle, int screenWidth, int screenHeight, bool fullscreen, bool verticalSync );

        void reportLiveObjects();

        void renderTexture( const Texture2DSpecBind<TexBind::ShaderResource, unsigned char >&  texture, float posX, float posY, float width, float height, bool blend );
        void renderTexture( const Texture2DSpecBind<TexBind::ShaderResource, uchar4 >&         texture, float posX, float posY, float width, float height, bool blend );
        void renderTexture( const Texture2DSpecBind<TexBind::ShaderResource, float4 >&         texture, float posX, float posY, float width, float height, bool blend );
        void renderTexture( const Texture2DSpecBind<TexBind::ShaderResource, float2  >&        texture, float posX, float posY, float width, float height, bool blend );
        void renderTexture( const Texture2DSpecBind<TexBind::ShaderResource, float  >&         texture, float posX, float posY, float width, float height, bool blend );

        void renderTextureAlpha( const Texture2DSpecBind<TexBind::ShaderResource, uchar4 >& texture, float posX, float posY, float width, float height, bool blend );

        void displayFrame();

        Microsoft::WRL::ComPtr< ID3D11Device > getDevice();
        Microsoft::WRL::ComPtr< ID3D11DeviceContext > getDeviceContext();

        private:

        Direct3DRendererCore& m_rendererCore;

        // Initalization.
        std::tuple<int, int>    getRefreshRateNumeratorDenominator( IDXGIAdapter& adapter, unsigned int screenWidth, unsigned int screenHeight );
        std::string             getGpuDescription( IDXGIAdapter& adapter );
        size_t                  getGpuMemory( IDXGIAdapter& adapter );

        std::tuple< Microsoft::WRL::ComPtr<IDXGISwapChain>, Microsoft::WRL::ComPtr<ID3D11Device>, Microsoft::WRL::ComPtr<ID3D11DeviceContext> >
            createDeviceAndSwapChain( HWND windowHandle, bool fullscreen, bool verticalSync, unsigned int screenWidth, unsigned int screenHeight, int refreshRateNumerator, int refreshRateDenominator );

        Microsoft::WRL::ComPtr<ID3D11Texture2D>       getBackbufferTexture( IDXGISwapChain& swapChain );
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> createRasterizerState( ID3D11Device& device );
        Microsoft::WRL::ComPtr<ID3D11BlendState>      createBlendStateNoBlending( ID3D11Device& device );
        Microsoft::WRL::ComPtr<ID3D11BlendState>      createBlendStateWithBlending( ID3D11Device& device );

        bool m_initialized;

        bool m_fullscreen;
        int m_screenWidth, m_screenHeight;
        bool m_verticalSync;

        size_t m_gpuMemory;
        std::string m_gpuDescription;

        Microsoft::WRL::ComPtr<ID3D11Device>          m_device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext>   m_deviceContext;
        Microsoft::WRL::ComPtr<IDXGISwapChain>        m_swapChain;
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
        Microsoft::WRL::ComPtr<ID3D11BlendState>      m_blendStateNoBlending;
        Microsoft::WRL::ComPtr<ID3D11BlendState>      m_blendStateWithBlending;

        std::shared_ptr< Texture2D< TexUsage::Default, TexBind::RenderTarget, uchar4 > > m_renderTarget;

        // Default mesh.
        RectangleMesh rectangleMesh;

        // Shaders.
        void loadAndCompileShaders( ID3D11Device& device );

        std::shared_ptr<TextureVertexShader>   m_textureVertexShader;
        std::shared_ptr<TextureFragmentShader> m_textureFragmentShader;
        std::shared_ptr<TextureFragmentShader> m_textureAlphaFragmentShader;
        std::shared_ptr<TextVertexShader>      m_textVertexShader;
        std::shared_ptr<TextFragmentShader>    m_textFragmentShader;

        // Copying is not allowed.
        Direct3DFrameRenderer( const Direct3DFrameRenderer& ) = delete;
        Direct3DFrameRenderer& operator=(const Direct3DFrameRenderer&) = delete;
    };
}

