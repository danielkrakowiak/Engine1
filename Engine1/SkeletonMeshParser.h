#pragma once

#include <vector>
#include <memory>

class SkeletonMesh;

class SkeletonMeshParser
{
	friend class SkeletonMesh;

	private:
	static std::shared_ptr<SkeletonMesh> parseFileInfoBinary( std::vector<unsigned char>::const_iterator& dataIt );
	static void                          writeFileInfoBinary( std::vector<unsigned char>& data, const SkeletonMesh& mesh );
};

