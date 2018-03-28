#include "ASSAORenderer.h"

#include "ASSAOCoreRenderer.h"
#include "TextFile.h"

#include "Settings.h"

using namespace Engine1;
using Microsoft::WRL::ComPtr;

ASSAORenderer::ASSAORenderer() :
    m_initialized( false )
{}

ASSAORenderer::~ASSAORenderer()
{}

void ASSAORenderer::initialize( ComPtr< ID3D11Device3 > device, 
                                ComPtr< ID3D11DeviceContext3 > deviceContext )
{
    this->m_device        = device;
	this->m_deviceContext = deviceContext;

    WCHAR directory[200];
    GetCurrentDirectory(200, (LPWSTR)&directory);

    const auto shaderText = TextFile::load( "Engine1/Shaders/ASSAOShader/ASSAO.hlsl" );

    const ASSAO_CreateDescDX11 desc( device.Get(), shaderText->data(), shaderText->size() );

    m_effect = std::shared_ptr< ASSAO_Effect >( ASSAO_Effect::CreateInstance( &desc ), ASSAO_Effect::DestroyInstance );

	m_initialized = true;
}

void ASSAORenderer::renderAmbientOcclusion( 
    std::shared_ptr< Texture2DSpecBind< TexBind::RenderTarget, unsigned char > > destTexture,
    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, float4 > > normalTexture,
    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > depthTexture,
    const float44& projectionMatrix,
    const float44& worldToViewspaceMatrix
    )
{
    if ( !m_initialized ) 
        throw std::exception( "ASSAORenderer::renderAmbientOcclusion - renderer not initialized." );

    // More info here: https://software.intel.com/en-us/articles/adaptive-screen-space-ambient-occlusion

    ASSAO_InputsDX11 inputs;
    inputs.ViewportWidth                 = settings().main.screenDimensions.x;
    inputs.ViewportHeight                = settings().main.screenDimensions.y;
    inputs.ProjectionMatrix              = *reinterpret_cast<const ASSAO_Float4x4*>(&projectionMatrix);
    inputs.NormalsWorldToViewspaceMatrix = *reinterpret_cast<const ASSAO_Float4x4*>(&worldToViewspaceMatrix);
    inputs.MatricesRowMajorOrder         = true;
    inputs.NormalsUnpackMul              = 1.0f;
    inputs.NormalsUnpackAdd              = 0.0f;
    inputs.DrawOpaque                    = true;
    inputs.DeviceContext                 = m_deviceContext.Get();
    inputs.DepthSRV                      = depthTexture->getShaderResourceView();
    inputs.NormalSRV                     = normalTexture->getShaderResourceView();
    inputs.OverrideOutputRTV             = destTexture->getRenderTargetView();

    ASSAO_Settings assaoSettings;
    assaoSettings.Radius                              = settings().rendering.ambientOcclusion.assao.radius;
    assaoSettings.ShadowMultiplier                    = settings().rendering.ambientOcclusion.assao.shadowMultiplier;
    assaoSettings.ShadowPower                         = settings().rendering.ambientOcclusion.assao.shadowPower;
    assaoSettings.ShadowClamp                         = settings().rendering.ambientOcclusion.assao.shadowClamp;
    assaoSettings.HorizonAngleThreshold               = settings().rendering.ambientOcclusion.assao.horizonAngleThreshold;
    assaoSettings.FadeOutFrom                         = settings().rendering.ambientOcclusion.assao.fadeOutFrom;
    assaoSettings.FadeOutTo                           = settings().rendering.ambientOcclusion.assao.fadeOutTo;
    assaoSettings.AdaptiveQualityLimit                = settings().rendering.ambientOcclusion.assao.adaptiveQualityLimit;
    assaoSettings.QualityLevel                        = settings().rendering.ambientOcclusion.assao.qualityLevel;
    assaoSettings.BlurPassCount                       = settings().rendering.ambientOcclusion.assao.blurPassCount;
    assaoSettings.Sharpness                           = settings().rendering.ambientOcclusion.assao.sharpness;
    assaoSettings.TemporalSupersamplingAngleOffset    = settings().rendering.ambientOcclusion.assao.temporalSupersamplingAngleOffset;
    assaoSettings.TemporalSupersamplingRadiusOffset   = settings().rendering.ambientOcclusion.assao.temporalSupersamplingRadiusOffset;
    assaoSettings.DetailShadowStrength                = settings().rendering.ambientOcclusion.assao.detailShadowStrength;

    m_effect->Draw( assaoSettings, &inputs );
}