#pragma once

#include <memory>

#include "float4.h"
#include "Texture2D.h"

class ModelTexture2D {

	public:

	ModelTexture2D( std::shared_ptr<Texture2D>& texture, int texcoordIndex = 0, float4 colorMultiplier = float4(1.0f, 1.0f, 1.0f, 1.0f) );
	ModelTexture2D( const ModelTexture2D& );
	~ModelTexture2D();

	std::shared_ptr<Texture2D> texture;
	int texcoordIndex;
	float4 colorMultiplier;

	private:

	// Copying is not allowed.
	ModelTexture2D& operator=( const ModelTexture2D& ) = delete;
};

