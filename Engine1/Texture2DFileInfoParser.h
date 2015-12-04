#pragma once

#include <vector>
#include <memory>

namespace Engine1
{
    class Texture2DFileInfo;

    class Texture2DFileInfoParser
    {
        friend class Texture2DFileInfo;

        private:
        static std::shared_ptr<Texture2DFileInfo> parseBinary( std::vector<char>::const_iterator& dataIt );
        static void                               writeBinary( std::vector<char>& data, const Texture2DFileInfo& fileInfo );
    };
}

