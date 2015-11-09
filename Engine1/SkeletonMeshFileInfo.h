#pragma once

#include <string>
#include <vector>
#include <memory>

class SkeletonMeshFileInfo
{
	public:

	enum class Format : char
	{
		DAE = 0
	};

	static std::shared_ptr<SkeletonMeshFileInfo> parseBinary( std::vector<unsigned char>::const_iterator& dataIt );

	SkeletonMeshFileInfo( );
	SkeletonMeshFileInfo::SkeletonMeshFileInfo( std::string path, Format format, int indexInFile = 0, bool invertZCoordinate = false, bool invertVertexWindingOrder = false, bool flipUVs = false );
	~SkeletonMeshFileInfo( );

	void writeBinary( std::vector<unsigned char>& data ) const;

	void setPath( std::string path );
	void setFormat( Format format );
	void setIndexInFile( int indexInFile );
	void setInvertZCoordinate( bool invertZCoordinate );
	void setInvertVertexWindingOrder( bool invertVertexWindingOrder );
	void setFlipUVs( bool flipUVs );

	std::string getPath() const;
	Format      getFormat() const;
	int         getIndexInFile() const;
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

