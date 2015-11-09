#pragma once

#include <vector>
#include <memory>

class ModelTexture2D;

class ModelTexture2DParser
{
	friend class ModelTexture2D;

	private:
	static std::shared_ptr<ModelTexture2D> parseBinary( std::vector<unsigned char>::const_iterator& dataIt, const bool loadRecurrently );
	static void                            writeBinary( std::vector<unsigned char>& data, const ModelTexture2D& modelTexture );
};

