#include "VertexShader.h"

#include <d3d11.h>
//#include <d3dx10math.h>
//#include <d3dcompiler.h>

using namespace Engine1;

unsigned int VertexShader::compiledShadersCount = 0;

VertexShader::VertexShader() : 
	m_compiled( false ),
	m_shaderId( 0 ) 
{}

VertexShader::~VertexShader()
{}

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

