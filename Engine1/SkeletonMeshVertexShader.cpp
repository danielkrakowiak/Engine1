#include "SkeletonMeshVertexShader.h"

#include "SkeletonMeshEnums.h"

#include "StringUtil.h"
#include "Direct3DUtil.h"

#include <d3d11.h>
#include <d3dx11async.h>

using Microsoft::WRL::ComPtr;

SkeletonMeshVertexShader::SkeletonMeshVertexShader( ) :
	bonesPerVertexCurrentConfig( BonesPerVertexCount::Type::ZERO )
{}

SkeletonMeshVertexShader::~SkeletonMeshVertexShader( ) {}

void SkeletonMeshVertexShader::compileFromFile( std::string path, ID3D11Device& device ) {
	if ( compiled ) throw std::exception( "SkeletonMeshVertexShader::compileFromFile - Shader has already been compiled" );

	HRESULT result;
	ComPtr<ID3D10Blob> shaderBuffer;
	{ //compile shader
		ComPtr<ID3D10Blob> errorMessage;

		UINT flags = D3D10_SHADER_ENABLE_STRICTNESS;

		#if defined(DEBUG_DIRECT3D) || defined(_DEBUG)
		flags |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
		#endif

		result = D3DX11CompileFromFile( StringUtil::widen( path ).c_str( ), nullptr, nullptr, "main", "vs_5_0", flags, 0, nullptr,
										shaderBuffer.GetAddressOf(), errorMessage.GetAddressOf(), nullptr );
		if ( result < 0 ) {
			if ( errorMessage ) {
				std::string compileMessage((char*)( errorMessage->GetBufferPointer( ) ));

				throw std::exception( ( std::string( "SkeletonMeshVertexShader::compileFromFile - Compilation failed with errors: " ) + compileMessage ).c_str( ) );
			} else {
				throw std::exception( "SkeletonMeshVertexShader::compileFromFile - Failed to open file" );
			}
		}

		result = device.CreateVertexShader( shaderBuffer->GetBufferPointer( ), shaderBuffer->GetBufferSize( ), nullptr, shader.GetAddressOf() );
		if ( result < 0 ) throw std::exception( "SkeletonMeshVertexShader::compileFromFile - Failed to create shader" );
	}

	// Create input layouts for each possible value of bones-per-vertex-count.
	for ( BonesPerVertexCount::Type bonesPerVertexCount : BonesPerVertexCount::correctValues )
	{
		DXGI_FORMAT boneIndicesFormat = DXGI_FORMAT_UNKNOWN;
		DXGI_FORMAT boneWeightsFormat = DXGI_FORMAT_UNKNOWN;

		if ( bonesPerVertexCount == BonesPerVertexCount::Type::ONE ) 
		{
			boneIndicesFormat = DXGI_FORMAT_R8_UINT;
			boneWeightsFormat = DXGI_FORMAT_R32_FLOAT;
		} 
		else if ( bonesPerVertexCount == BonesPerVertexCount::Type::TWO ) 
		{
			boneIndicesFormat = DXGI_FORMAT_R8G8_UINT;
			boneWeightsFormat = DXGI_FORMAT_R32G32_FLOAT;
		} 
		else if ( bonesPerVertexCount == BonesPerVertexCount::Type::FOUR ) 
		{
			boneIndicesFormat = DXGI_FORMAT_R8G8B8A8_UINT;
			boneWeightsFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		const int inputLayoutCount = 4;
		D3D11_INPUT_ELEMENT_DESC inputLayoutDesc[ inputLayoutCount ];

		inputLayoutDesc[ 0 ].SemanticName         = "POSITION";
		inputLayoutDesc[ 0 ].SemanticIndex        = 0;
		inputLayoutDesc[ 0 ].Format               = DXGI_FORMAT_R32G32B32_FLOAT;
		inputLayoutDesc[ 0 ].InputSlot            = 0;
		inputLayoutDesc[ 0 ].AlignedByteOffset    = 0;
		inputLayoutDesc[ 0 ].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
		inputLayoutDesc[ 0 ].InstanceDataStepRate = 0;

		inputLayoutDesc[ 1 ].SemanticName         = "BLENDINDICES";
		inputLayoutDesc[ 1 ].SemanticIndex        = 0;
		inputLayoutDesc[ 1 ].Format               = boneIndicesFormat;
		inputLayoutDesc[ 1 ].InputSlot            = 1;
		inputLayoutDesc[ 1 ].AlignedByteOffset    = 0;
		inputLayoutDesc[ 1 ].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
		inputLayoutDesc[ 1 ].InstanceDataStepRate = 0;

		inputLayoutDesc[ 2 ].SemanticName         = "BLENDWEIGHT";
		inputLayoutDesc[ 2 ].SemanticIndex        = 0;
		inputLayoutDesc[ 2 ].Format               = boneWeightsFormat;
		inputLayoutDesc[ 2 ].InputSlot            = 2;
		inputLayoutDesc[ 2 ].AlignedByteOffset    = 0;
		inputLayoutDesc[ 2 ].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
		inputLayoutDesc[ 2 ].InstanceDataStepRate = 0;
		
		inputLayoutDesc[ 3 ].SemanticName         = "NORMAL";
		inputLayoutDesc[ 3 ].SemanticIndex        = 0;
		inputLayoutDesc[ 3 ].Format               = DXGI_FORMAT_R32G32B32_FLOAT;
		inputLayoutDesc[ 3 ].InputSlot            = 3;
		inputLayoutDesc[ 3 ].AlignedByteOffset    = 0;
		inputLayoutDesc[ 3 ].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
		inputLayoutDesc[ 3 ].InstanceDataStepRate = 0;

		ComPtr<ID3D11InputLayout> inputLayout;

		// Create the vertex input layout.
		result = device.CreateInputLayout( inputLayoutDesc, inputLayoutCount, shaderBuffer->GetBufferPointer( ),
										   shaderBuffer->GetBufferSize( ), inputLayout.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "SkeletonMeshVertexShader::compileFromFile - creating input layout failed." );

		// Save input layout.
		inputLayouts.insert( std::make_pair( bonesPerVertexCount, inputLayout ) );
	}

	{
		// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
		D3D11_BUFFER_DESC matrixBufferDesc;
		matrixBufferDesc.Usage               = D3D11_USAGE_DYNAMIC;
		matrixBufferDesc.ByteWidth           = sizeof( ConstantBuffer );
		matrixBufferDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
		matrixBufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
		matrixBufferDesc.MiscFlags           = 0;
		matrixBufferDesc.StructureByteStride = 0;

		// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
		result = device.CreateBuffer( &matrixBufferDesc, nullptr, constantInputBuffer.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "SkeletonMeshVertexShader::compileFromFile - creating constant input buffer failed" );

		#if defined(DEBUG_DIRECT3D) || defined(_DEBUG) 
		std::string resourceName = std::string( "SkeletonMeshVertexShader::constantInputBuffer" );
		Direct3DUtil::setResourceName( *constantInputBuffer.Get(), resourceName );
		#endif
	}

	this->device   = &device;
	this->compiled = true;
	this->shaderId = ++compiledShadersCount;
}

void SkeletonMeshVertexShader::setParameters( ID3D11DeviceContext& deviceContext, const float43& worldMatrix, const float44& viewMatrix, const float44& projectionMatrix, const SkeletonMesh& skeletonMesh, const SkeletonPose& skeletonPoseInSkeletonSpace ) {
	if ( !compiled ) throw std::exception( "SkeletonMeshVertexShader::setParameters - Shader hasn't been compiled yet." );
	if ( skeletonMesh.getBoneCount( ) != skeletonPoseInSkeletonSpace.getBonesCount( ) ) throw std::exception( "SkeletonMeshVertexShader::setParameters - there is different number of bones in bind pose and current pose." );
	if ( skeletonMesh.getBoneCount( ) > maxBoneCount ) throw std::exception( "SkeletonMeshVertexShader::setParameters - the number of bones in the mesh exceeds the shader limit." );
	if ( skeletonMesh.getBonesPerVertexCount() == BonesPerVertexCount::Type::ZERO ) throw std::exception( "SkeletonMeshVertexShader::setParameters - mesh's number-of-bones-per-vertex is ZERO. Should be one of the supported positive values." );

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	ConstantBuffer* dataPtr;

	HRESULT result = deviceContext.Map( constantInputBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
	if ( result < 0 ) throw std::exception( "SkeletonMeshVertexShader::setParameters - mapping shader's constant input buffer to RAM memory failed" );

	dataPtr = (ConstantBuffer*)mappedResource.pData;

	{ // Fill constant buffer.
		dataPtr->world      = float44( worldMatrix ).getTranspose();
		dataPtr->view       = viewMatrix.getTranspose();
		dataPtr->projection = projectionMatrix.getTranspose();

		const unsigned char boneCount = skeletonMesh.getBoneCount();
		for ( unsigned char boneIndex = 1; boneIndex <= boneCount; ++boneIndex ) 
		{
			dataPtr->bonesBindPose[ boneIndex - 1 ]    = float44( skeletonMesh.getBone( boneIndex ).getBindPose() ).getTranspose();
			dataPtr->bonesBindPoseInv[ boneIndex - 1 ] = float44( skeletonMesh.getBone( boneIndex ).getBindPoseInv() ).getTranspose();
			dataPtr->bonesPose[ boneIndex - 1 ]        = float44( skeletonPoseInSkeletonSpace.getBonePose( boneIndex ) ).getTranspose();
		}

		// Set unused bones' pose to identity.
		for ( unsigned char i = boneCount; i < maxBoneCount; ++i ) 
		{
			dataPtr->bonesBindPose[ i ].identity();
			dataPtr->bonesBindPoseInv[ i ].identity();
			dataPtr->bonesPose[ i ].identity();
		}

		dataPtr->bonesPerVertex = static_cast<unsigned char>( skeletonMesh.getBonesPerVertexCount() );
	}

	deviceContext.Unmap( constantInputBuffer.Get(), 0 );

	deviceContext.VSSetConstantBuffers( 0, 1, constantInputBuffer.GetAddressOf() );

	// Save currently configured bones-per-vertex-count.
	bonesPerVertexCurrentConfig = skeletonMesh.getBonesPerVertexCount();
}

ID3D11InputLayout& SkeletonMeshVertexShader::getInputLauout( ) const
{
	if ( !compiled ) throw std::exception( "SkeletonMeshVertexShader::getInputLauout() - Shader hasn't been compiled yet." );
	if ( bonesPerVertexCurrentConfig == BonesPerVertexCount::Type::ZERO ) throw std::exception( "SkeletonMeshVertexShader::getInputLauout() - Shader isn't configured properly. Currently set number-of-bones-per-vertex is ZERO. Should be one of the supported positive values." );

	std::map< BonesPerVertexCount::Type, ComPtr<ID3D11InputLayout> >::const_iterator inputLayoutIt;

	inputLayoutIt = inputLayouts.find( bonesPerVertexCurrentConfig );

	if ( inputLayoutIt != inputLayouts.end() )
		return *inputLayoutIt->second.Get();
	else
		throw std::exception( "SkeletonMeshVertexShader::getInputLauout() - Failed to find 'input layout' matching bones-per-vertex value." );
}