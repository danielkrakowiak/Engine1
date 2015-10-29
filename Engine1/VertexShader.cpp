#include "VertexShader.h"

#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>

unsigned int VertexShader::compiledShadersCount = 0;

VertexShader::VertexShader() : 
	compiled( false ),
	shaderId( 0 ) 
{}

VertexShader::~VertexShader()
{}

bool VertexShader::isSame( const VertexShader& shader ) const
{
	if ( !compiled || !shader.compiled ) throw std::exception( "VertexShader::isSame - One of the compared shaders hasn't been compiled yet." );

	return shaderId == shader.shaderId;
}

ID3D11VertexShader& VertexShader::getShader() const
{
	if ( !compiled ) throw std::exception( "VertexShader::getShader() - Shader hasn't been compiled yet." );

	return *shader.Get();
}

bool VertexShader::isCompiled() const
{
	return compiled;
}

