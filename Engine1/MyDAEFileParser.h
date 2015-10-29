#pragma once

#include <vector>
#include <memory>

class BlockMesh;
class SkeletonMesh;

class MyDAEFileParser {
private:
	MyDAEFileParser( );
	~MyDAEFileParser( );

public:
	static std::vector< std::shared_ptr<BlockMesh> >    parseBlockMeshFile( const std::vector<char>& file, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs );
	static std::vector< std::shared_ptr<SkeletonMesh> > parseSkeletonMeshFile( const std::vector<char>& file, const bool invertZCoordinate, const bool invertVertexWindingOrder, const bool flipUVs );
};

