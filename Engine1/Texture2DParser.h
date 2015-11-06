#pragma once

#include <vector>
#include <memory>

class Texture2D;

class Texture2DParser
{
	friend class Texture2D;

	private:
	static std::shared_ptr<Texture2D> parseFileInfoBinary( std::vector<unsigned char>::const_iterator& dataIt );
	static void writeFileInfoBinary( std::vector<unsigned char>& data, const Texture2D& texture );
};

