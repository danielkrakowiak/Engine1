#pragma once

#include <string>
#include <vector>
#include <memory>

#include "FileInfo.h"

namespace Engine1
{
    class Texture2DFileInfo : public FileInfo
    {
        public:

        enum class Format : char
        {
            BMP = 0,
            DDS = 1,
            JPEG = 2,
            PNG = 3,
            RAW = 4,
            TIFF = 5,
            TGA = 6
        };

        static std::shared_ptr<Texture2DFileInfo> parseBinary( std::vector<char>::const_iterator& dataIt );

        Texture2DFileInfo();
        Texture2DFileInfo( std::string path, Format format );
        ~Texture2DFileInfo();

        std::shared_ptr<FileInfo> clone() const;

        void writeBinary( std::vector<char>& data ) const;

        void setPath( std::string path );
        void setFormat( Format format );

        std::string getPath() const;
        int         getIndexInFile() const;
        Asset::Type getAssetType() const;
        FileType    getFileType() const;
        Format      getFormat() const;

        private:

        std::string path;
        Format format;
    };
}

