#pragma once

#include "FragmentShader.h"

class BlockMeshFragmentShader :	public FragmentShader {
	public:

	BlockMeshFragmentShader( );
	virtual ~BlockMeshFragmentShader( );

	void compileFromFile( std::string path, ID3D11Device& device );

	private:

	// Copying is not allowed.
	BlockMeshFragmentShader( const BlockMeshFragmentShader& ) = delete;
	BlockMeshFragmentShader& operator=( const BlockMeshFragmentShader& ) = delete;
};

