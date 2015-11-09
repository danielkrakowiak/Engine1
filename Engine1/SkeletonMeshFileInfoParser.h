#pragma once

#include <vector>
#include <memory>

class SkeletonMeshFileInfo;

class SkeletonMeshFileInfoParser
{
	friend class SkeletonMeshFileInfo;

	private:
	static std::shared_ptr<SkeletonMeshFileInfo> parseBinary( std::vector<unsigned char>::const_iterator& dataIt );
	static void                                  writeBinary( std::vector<unsigned char>& data, const SkeletonMeshFileInfo& fileInfo );
};

