#pragma once

#include <vector>
#include <memory>

class BlockMeshFileInfo;

class BlockMeshFileInfoParser
{
	friend class BlockMeshFileInfo;

	private:
	static std::shared_ptr<BlockMeshFileInfo> parseBinary( std::vector<char>::const_iterator& dataIt );
	static void                               writeBinary( std::vector<char>& data, const BlockMeshFileInfo& fileInfo );
};

