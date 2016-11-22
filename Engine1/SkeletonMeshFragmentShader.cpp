#include "SkeletonMeshFragmentShader.h"

#include "StringUtil.h"

#include <d3d11.h>
#include <d3dcompiler.h>

using namespace Engine1;

using Microsoft::WRL::ComPtr;

SkeletonMeshFragmentShader::SkeletonMeshFragmentShader() {}

SkeletonMeshFragmentShader::~SkeletonMeshFragmentShader() {}

void SkeletonMeshFragmentShader::initialize( ComPtr< ID3D11Device >& device )
{
    device;
}
