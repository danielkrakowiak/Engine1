#include "FragmentShader.h"

#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>

using namespace Engine1;

unsigned int FragmentShader::compiledShadersCount = 0;

FragmentShader::FragmentShader() :
compiled( false ),
shaderId( 0 )
{}

FragmentShader::~FragmentShader()
{}

bool FragmentShader::isSame( const FragmentShader& shader ) const
{
	if ( !compiled || !shader.compiled ) throw std::exception( "FragmentShader::isSame - One of the compared shader hasn't been compiled yet." );

	return shaderId == shader.shaderId;
}

ID3D11PixelShader& FragmentShader::getShader() const
{
	if ( !compiled ) throw std::exception( "FragmentShader::getShader - Shader hasn't been compiled yet." );

	return *shader.Get();
}

bool FragmentShader::isCompiled() const
{
	return compiled;
}

