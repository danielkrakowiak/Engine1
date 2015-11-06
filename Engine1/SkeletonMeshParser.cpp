#include "SkeletonMeshParser.h"

#include "BinaryFile.h"
#include "SkeletonMesh.h"

std::shared_ptr<SkeletonMesh> SkeletonMeshParser::parseFileInfoBinary( std::vector<unsigned char>::const_iterator& dataIt )
{
	std::shared_ptr<SkeletonMesh> mesh = std::make_shared<SkeletonMesh>( );

	const int filePathSize = BinaryFile::readInt( dataIt );
	mesh->filePath = BinaryFile::readText( dataIt, filePathSize );
	mesh->indexInFile = BinaryFile::readInt( dataIt );
	mesh->fileFormat = static_cast<SkeletonMesh::FileFormat>( BinaryFile::readInt( dataIt ) );
	mesh->fileInvertedZCoordinate = BinaryFile::readBool( dataIt );
	mesh->fileInvertedVertexWindingOrder = BinaryFile::readBool( dataIt );
	mesh->fileFlipedUVs = BinaryFile::readBool( dataIt );

	return mesh;
}

void SkeletonMeshParser::writeFileInfoBinary( std::vector<unsigned char>& data, const SkeletonMesh& mesh )
{
	BinaryFile::writeInt( data, mesh.getFilePath().size() );
	BinaryFile::writeText( data, mesh.getFilePath() );
	BinaryFile::writeInt( data, mesh.getIndexInFile() );
	BinaryFile::writeInt( data, static_cast<int>( mesh.getFileFormat() ) );
	BinaryFile::writeBool( data, mesh.getFileInvertedZCoordinate() );
	BinaryFile::writeBool( data, mesh.getFileInvertedVertexWindingOrder() );
	BinaryFile::writeBool( data, mesh.getFileFlipedUVs() );
}
