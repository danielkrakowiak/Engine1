#include "FragmentShader.h"

#include <d3d11_3.h>
//#include <d3dcompiler.h>

//#include "StringUtil.h"
#include "BinaryFile.h"

using namespace Engine1;

using Microsoft::WRL::ComPtr;

unsigned int FragmentShader::compiledShadersCount = 0;

FragmentShader::FragmentShader() :
m_compiled( false ),
m_shaderId( 0 )
{}

FragmentShader::~FragmentShader()
{}

void FragmentShader::loadAndInitialize( const std::string& path, ComPtr< ID3D11Device3 >& device )
{
    load( path, device );
    initialize( device );
}

void FragmentShader::load( const std::string& path, ComPtr< ID3D11Device3 >& device )
{
    if ( m_compiled )
        throw std::exception( "FragmentShader::loadFromFile - Shader has already been compiled" );

    const auto compiledShader = BinaryFile::load( path );

    HRESULT result = device->CreatePixelShader( compiledShader->data(), compiledShader->size(), nullptr, m_shader.ReleaseAndGetAddressOf() );
    if ( result < 0 )
        throw std::exception( "FragmentShader::loadFromFile - Failed to create shader" );

    this->m_device   = device;
    this->m_compiled = true;
    this->m_shaderId = ++compiledShadersCount;
}

void FragmentShader::initialize( ComPtr< ID3D11Device3 >& device )
{
    device;
}

bool FragmentShader::isSame( const FragmentShader& shader ) const
{
	if ( !m_compiled || !shader.m_compiled ) throw std::exception( "FragmentShader::isSame - One of the compared shader hasn't been compiled yet." );

	return m_shaderId == shader.m_shaderId;
}

ID3D11PixelShader& FragmentShader::getShader() const
{
	if ( !m_compiled ) throw std::exception( "FragmentShader::getShader - Shader hasn't been compiled yet." );

	return *m_shader.Get();
}

bool FragmentShader::isCompiled() const
{
	return m_compiled;
}

unsigned int FragmentShader::getCompileFlags() const
{
    //#TODO: Is that flag still needed?
    UINT flags = D3D10_SHADER_ENABLE_STRICTNESS;

#if defined(_DEBUG)
    flags |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

    return flags;
}

