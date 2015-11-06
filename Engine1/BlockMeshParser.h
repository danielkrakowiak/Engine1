#pragma once

#include <vector>
#include <memory>

class BlockMesh;

class BlockMeshParser
{
	friend class BlockMesh;

	private:
	static std::shared_ptr<BlockMesh> parseFileInfoBinary( std::vector<unsigned char>::const_iterator& dataIt );
	static void                       writeFileInfoBinary( std::vector<unsigned char>& data, const BlockMesh& mesh );
};

