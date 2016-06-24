#pragma once

#include <string>
#include <vector>
#include <memory>

#include "FileInfo.h"

namespace Engine1
{
    class SkeletonModelFileInfo : public FileInfo
    {
        public:

        enum class Format : char
        {
            SKELETONMODEL = 0
        };

        static std::shared_ptr<SkeletonModelFileInfo> createFromMemory( std::vector<char>::const_iterator& dataIt );

        SkeletonModelFileInfo();
        SkeletonModelFileInfo( std::string path, Format format, int indexInFile = 0 );
        SkeletonModelFileInfo( const SkeletonModelFileInfo& obj );
        ~SkeletonModelFileInfo();

        std::shared_ptr<FileInfo> clone() const;

        void saveToMemory( std::vector<char>& data ) const;

        void setPath( std::string path );
        void setFormat( Format format );

        Asset::Type getAssetType() const;
        FileType    getFileType() const;
        bool        canHaveSubAssets() const;
        std::string getPath() const;
        Format      getFormat() const;
        int         getIndexInFile() const;

        private:

        std::string path;
        Format format;
    };
}

