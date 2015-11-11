#pragma once

#include <memory>

#include "float4.h"
#include "Texture2D.h"

class ModelTexture2D {

	public:

	static std::shared_ptr< ModelTexture2D > createFromBinary( std::vector<char>::const_iterator& dataIt, const bool loadRecurrently );

	void writeBinary( std::vector<char>& data ) const;

	ModelTexture2D();
	ModelTexture2D( std::shared_ptr<Texture2D> texture, int texcoordIndex = 0, float4 colorMultiplier = float4(1.0f, 1.0f, 1.0f, 1.0f) );
	ModelTexture2D( const ModelTexture2D& );
	~ModelTexture2D();

	ModelTexture2D& operator=( const ModelTexture2D& );

	const std::shared_ptr<Texture2D> getTexture() const;
	std::shared_ptr<Texture2D> getTexture();
	int getTexcoordIndex() const;
	float4 getColorMultiplier() const;

	void setTexture( std::shared_ptr<Texture2D> texture );
	void setTexcoordIndex( int texcoordIndex );
	void setColorMultiplier( float4 colorMultiplier );

	private:

	std::shared_ptr<Texture2D> texture;
	int texcoordIndex;
	float4 colorMultiplier;

	
};

