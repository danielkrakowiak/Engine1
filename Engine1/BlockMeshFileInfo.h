#pragma once

#include <string>
#include <vector>
#include <memory>

class BlockMeshFileInfo
{
	public:

	enum class Format : char
	{
		OBJ = 0,
		DAE = 1
	};

	static std::shared_ptr<BlockMeshFileInfo> parseBinary( std::vector<unsigned char>::const_iterator& dataIt );

	BlockMeshFileInfo();
	BlockMeshFileInfo::BlockMeshFileInfo( std::string path, Format format, int indexInFile = 0, bool invertZCoordinate = false, bool invertVertexWindingOrder = false, bool flipUVs = false );
	~BlockMeshFileInfo();

	void writeBinary( std::vector<unsigned char>& data ) const;

	void setPath( std::string path );
	void setFormat( Format format );
	void setIndexInFile( int indexInFile );
	void setInvertZCoordinate( bool invertZCoordinate );
	void setInvertVertexWindingOrder( bool invertVertexWindingOrder );
	void setFlipUVs( bool flipUVs );

	std::string getPath() const;
	Format      getFormat() const;
	int         getIndexInFile( ) const;
	bool        getInvertZCoordinate() const;
	bool        getInvertVertexWindingOrder() const;
	bool        getFlipUVs() const;

	private:

	std::string path;
	Format format;
	int indexInFile;
	bool invertZCoordinate;
	bool invertVertexWindingOrder;
	bool flipUVs;
};

