#include "VertexShader.h"

#include <d3d11.h>
//#include <d3dcompiler.h>

#include "BinaryFile.h"
//#include "StringUtil.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

unsigned int VertexShader::compiledShadersCount = 0;

VertexShader::VertexShader() : 
	m_compiled( false ),
	m_shaderId( 0 ) 
{}

VertexShader::~VertexShader()
{}

void VertexShader::loadAndInitialize( const std::string& path, ComPtr< ID3D11Device >& device )
{
    load( path, device );
    initialize( device );
}

void VertexShader::load( const std::string& path, ComPtr< ID3D11Device >& device )
{
    if ( m_compiled )
        throw std::exception( "VertexShader::loadFromFile - Shader has already been compiled" );

    m_shaderBytecode = BinaryFile::load( path );

    // #TODO: Should I "cast" ID3D11Device to ID3D11Device3?

    HRESULT result = device->CreateVertexShader( m_shaderBytecode->data(), m_shaderBytecode->size(), nullptr, m_shader.ReleaseAndGetAddressOf() );
    if ( result < 0 )
        throw std::exception( "VertexShader::loadFromFile - Failed to create shader" );

    this->m_device   = device;
    this->m_compiled = true;
    this->m_shaderId = ++compiledShadersCount;
}

void VertexShader::initialize( ComPtr< ID3D11Device >& device )
{
    device;
}

bool VertexShader::isSame( const VertexShader& shader ) const
{
	if ( !m_compiled || !shader.m_compiled ) throw std::exception( "VertexShader::isSame - One of the compared shaders hasn't been compiled yet." );

	return m_shaderId == shader.m_shaderId;
}

ID3D11VertexShader& VertexShader::getShader() const
{
	if ( !m_compiled ) throw std::exception( "VertexShader::getShader() - Shader hasn't been compiled yet." );

	return *m_shader.Get();
}

bool VertexShader::isCompiled() const
{
	return m_compiled;
}

unsigned int VertexShader::getCompileFlags() const
{
    //#TODO: Is that flag still needed?
    UINT flags = D3D10_SHADER_ENABLE_STRICTNESS;

#if defined(_DEBUG)
    flags |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

    return flags;
}

