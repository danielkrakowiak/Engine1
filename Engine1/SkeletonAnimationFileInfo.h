#pragma once

#include <string>
#include <vector>
#include <memory>

#include "FileInfo.h"
#include "SkeletonMeshFileInfo.h"



namespace Engine1
{
    class SkeletonMesh;

    class SkeletonAnimationFileInfo : public FileInfo
    {
        public:

        enum class Format : char
        {
            XAF = 0
        };

        static std::shared_ptr<SkeletonAnimationFileInfo> createFromMemory( std::vector<char>::const_iterator& dataIt );

        SkeletonAnimationFileInfo();
        SkeletonAnimationFileInfo( std::string path, Format format, const SkeletonMeshFileInfo meshFileInfo, bool invertZCoordinate = false );
        ~SkeletonAnimationFileInfo();

        std::shared_ptr<FileInfo> clone() const;

        void saveToMemory( std::vector<char>& data ) const;

        void setPath( std::string path );
        void setFormat( Format format );
        void setMeshFileInfo( const SkeletonMeshFileInfo meshFileInfo );
        void setInvertZCoordinate( bool invertZCoordinate );


        Asset::Type          getAssetType() const;
        FileType             getFileType() const;
        bool                 canHaveSubAssets() const; 
        std::string          getPath() const;
        Format               getFormat() const;
        int                  getIndexInFile() const;
        SkeletonMeshFileInfo getMeshFileInfo() const;
        bool                 getInvertZCoordinate() const;

        private:

        std::string path;
        Format format;
        SkeletonMeshFileInfo meshFileInfo;
        bool invertZCoordinate;

    };
}

