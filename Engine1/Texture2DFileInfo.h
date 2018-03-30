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
            DDS,
            JPEG,
            PNG,
            RAW,
            TIFF,
            TGA,
            GIF
        };

        enum class PixelType : char
        {
            UCHAR  = 0,
            UCHAR4 = 1,
        };

        static Format fromExtension( const std::string& extension );

        static std::shared_ptr<Texture2DFileInfo> createFromMemory( std::vector<char>::const_iterator& dataIt );

        Texture2DFileInfo();
        Texture2DFileInfo( std::string path, Format format, PixelType pixelType );
        ~Texture2DFileInfo();

        std::shared_ptr<FileInfo> clone() const;

        void saveToMemory( std::vector<char>& data ) const;

        void setPath( std::string path );
        void setFormat( Format format );
        void setPixelType( PixelType pixelType );

        std::string getPath() const;
        int         getIndexInFile() const;
        Asset::Type getAssetType() const;
        FileType    getFileType() const;
        bool        canHaveSubAssets() const;
        Format      getFormat() const;
        PixelType   getPixelType() const;

        private:

        std::string m_path;
        Format      m_format;
        PixelType   m_pixelType;
    };
}

