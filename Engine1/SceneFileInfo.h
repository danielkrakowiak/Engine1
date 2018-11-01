#pragma once

#include <string>
#include <vector>
#include <memory>

#include "FileInfo.h"

namespace Engine1
{
    class SceneFileInfo : public FileInfo
    {
        public:

        SceneFileInfo();
        SceneFileInfo( std::string path );
        SceneFileInfo( const SceneFileInfo& obj );
        ~SceneFileInfo();

        std::shared_ptr<FileInfo> clone() const;

        void saveToMemory( std::vector<char>& data ) const;

        void setPath( std::string path );

        Asset::Type getAssetType() const;
        FileType    getFileType() const;
        bool        canHaveSubAssets() const; 
        std::string getPath() const;
        int         getIndexInFile() const;

        private:

        std::string m_path;
    };
}



