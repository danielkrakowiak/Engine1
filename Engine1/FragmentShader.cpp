#include "FragmentShader.h"

#include <d3d11.h>
#include <d3dcompiler.h>

using namespace Engine1;

unsigned int FragmentShader::compiledShadersCount = 0;

FragmentShader::FragmentShader() :
m_compiled( false ),
m_shaderId( 0 )
{}

FragmentShader::~FragmentShader()
{}

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

