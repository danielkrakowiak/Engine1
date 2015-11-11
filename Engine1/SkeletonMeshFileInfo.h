#pragma once

#include <string>
#include <vector>
#include <memory>

#include "FileInfo.h"

class SkeletonMeshFileInfo : public FileInfo
{
	public:

	enum class Format : char
	{
		DAE = 0
	};

	static std::shared_ptr<SkeletonMeshFileInfo> parseBinary( std::vector<char>::const_iterator& dataIt );

	SkeletonMeshFileInfo( );
	SkeletonMeshFileInfo( std::string path, Format format, int indexInFile = 0, bool invertZCoordinate = false, bool invertVertexWindingOrder = false, bool flipUVs = false );
	~SkeletonMeshFileInfo( );

	std::shared_ptr<FileInfo> clone( ) const;

	void writeBinary( std::vector<char>& data ) const;

	void setPath( std::string path );
	void setFormat( Format format );
	void setIndexInFile( int indexInFile );
	void setInvertZCoordinate( bool invertZCoordinate );
	void setInvertVertexWindingOrder( bool invertVertexWindingOrder );
	void setFlipUVs( bool flipUVs );

	Asset::Type getAssetType( ) const;
	FileType    getFileType() const;
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

