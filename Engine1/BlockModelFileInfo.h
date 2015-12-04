#pragma once

#include <string>
#include <vector>
#include <memory>

#include "FileInfo.h"

namespace Engine1
{
    class BlockModelFileInfo : public FileInfo
    {
        public:

        enum class Format : char
        {
            BLOCKMODEL = 0
        };

        //static std::shared_ptr<BlockModelFileInfo> parseBinary( std::vector<char>::const_iterator& dataIt );

        BlockModelFileInfo();
        BlockModelFileInfo( std::string path, Format format, int indexInFile = 0 );
        ~BlockModelFileInfo();

        std::shared_ptr<FileInfo> clone() const;

        //void writeBinary( std::vector<char>& data ) const;

        void setPath( std::string path );
        void setFormat( Format format );
        void setIndexInFile( int indexInFile );

        Asset::Type getAssetType() const;
        FileType    getFileType() const;
        std::string getPath() const;
        Format      getFormat() const;
        int         getIndexInFile() const;

        private:

        std::string path;
        Format format;
        int indexInFile;
    };
}

