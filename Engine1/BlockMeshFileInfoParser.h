#pragma once

#include <vector>
#include <memory>

class BlockMeshFileInfo;

class BlockMeshFileInfoParser
{
	friend class BlockMeshFileInfo;

	private:
	static std::shared_ptr<BlockMeshFileInfo> parseBinary( std::vector<unsigned char>::const_iterator& dataIt );
	static void                               writeBinary( std::vector<unsigned char>& data, const BlockMeshFileInfo& fileInfo );
};

