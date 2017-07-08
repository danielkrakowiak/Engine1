#include "ComputeShader.h"

#include <d3d11.h>
#include <d3d11_3.h>
//#include <d3dcompiler.h>

//#include "StringUtil.h"
#include "BinaryFile.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

unsigned int ComputeShader::compiledShadersCount = 0;

ComputeShader::ComputeShader() :
m_compiled( false ),
m_shaderId( 0 )
{}

ComputeShader::~ComputeShader()
{}

void ComputeShader::loadAndInitialize( const std::string& path, ComPtr< ID3D11Device >& device )
{
    load( path, device );
    initialize( device );
}

void ComputeShader::load( const std::string& path, ComPtr< ID3D11Device >& device )
{
    if ( m_compiled )
        throw std::exception( "ComputeShader::loadFromFile - Shader has already been compiled" );

    const auto compiledShader = BinaryFile::load( path );

    // #TODO: Should I "cast" ID3D11Device to ID3D11Device3?

    ComPtr< ID3D11Device3 > device3;

    device.As( &device3 );

    HRESULT result = device3->CreateComputeShader( compiledShader->data(), compiledShader->size(), nullptr, m_shader.ReleaseAndGetAddressOf() );
    if ( result < 0 )
        throw std::exception( "ComputeShader::loadFromFile - Failed to create shader" );

    this->m_device   = device;
    this->m_compiled = true;
    this->m_shaderId = ++compiledShadersCount;
}

void ComputeShader::initialize( ComPtr< ID3D11Device >& device )
{
    device;
}

bool ComputeShader::isSame( const ComputeShader& shader ) const
{
    if ( !m_compiled || !shader.m_compiled ) throw std::exception( "ComputeShader::isSame - One of the compared shaders hasn't been compiled yet." );

    return m_shaderId == shader.m_shaderId;
}

ID3D11ComputeShader& ComputeShader::getShader() const
{
    if ( !m_compiled ) throw std::exception( "ComputeShader::getShader() - Shader hasn't been compiled yet." );

    return *m_shader.Get();
}

bool ComputeShader::isCompiled() const
{
    return m_compiled;
}

unsigned int ComputeShader::getCompileFlags() const
{
    //#TODO: Is that flag still needed?
    UINT flags = D3D10_SHADER_ENABLE_STRICTNESS;

#if defined(_DEBUG)
    flags |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

    return flags;
}

