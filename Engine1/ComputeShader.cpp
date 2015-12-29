#include "ComputeShader.h"

#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>

using namespace Engine1;

unsigned int ComputeShader::compiledShadersCount = 0;

ComputeShader::ComputeShader() :
compiled( false ),
shaderId( 0 )
{}

ComputeShader::~ComputeShader()
{}

bool ComputeShader::isSame( const ComputeShader& shader ) const
{
    if ( !compiled || !shader.compiled ) throw std::exception( "ComputeShader::isSame - One of the compared shaders hasn't been compiled yet." );

    return shaderId == shader.shaderId;
}

ID3D11ComputeShader& ComputeShader::getShader() const
{
    if ( !compiled ) throw std::exception( "ComputeShader::getShader() - Shader hasn't been compiled yet." );

    return *shader.Get();
}

bool ComputeShader::isCompiled() const
{
    return compiled;
}

