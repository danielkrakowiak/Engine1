#pragma once

#include <string>
#include <vector>
#include <memory>

namespace Engine1
{
    class SkeletonModel;

    class SkeletonModelParser
    {
        friend class SkeletonModel;

        private:
        static std::shared_ptr<SkeletonModel> parseBinary( const std::vector<char>& data, const bool loadRecurrently );
        static void                           writeBinary( std::vector<char>& data, const SkeletonModel& model );

        static std::string fileTypeIdentifier;
    };
}

