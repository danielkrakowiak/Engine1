#pragma once

#include <string>
#include <vector>
#include <memory>

#include "FileInfo.h"

namespace Engine1
{
    class BlockMeshFileInfo : public FileInfo
    {
        public:

        enum class Format : char
        {
            OBJ = 0,
            DAE = 1,
            FBX = 2
        };

        static std::shared_ptr<BlockMeshFileInfo> createFromMemory( std::vector<char>::const_iterator& dataIt );

        BlockMeshFileInfo();
        BlockMeshFileInfo( std::string path, Format format, int indexInFile = 0, bool invertZCoordinate = false, bool invertVertexWindingOrder = false, bool flipUVs = false );
        ~BlockMeshFileInfo();

        std::shared_ptr<FileInfo> clone() const;

        void saveToMemory( std::vector<char>& data ) const;

        void setPath( std::string path );
        void setFormat( Format format );
        void setIndexInFile( int indexInFile );
        void setInvertZCoordinate( bool invertZCoordinate );
        void setInvertVertexWindingOrder( bool invertVertexWindingOrder );
        void setFlipUVs( bool flipUVs );

        Asset::Type getAssetType() const;
        FileType    getFileType() const;
        bool        canHaveSubAssets() const; 
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
}

