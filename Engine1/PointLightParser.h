#pragma once

#include <vector>
#include <memory>

namespace Engine1
{
    class PointLight;

    class PointLightParser
    {
        friend class PointLight;

        private:
        static std::shared_ptr<PointLight> parseBinary( std::vector< char >::const_iterator& dataIt );
        static void                        writeBinary( std::vector< char >& data, const PointLight& light );
    };
};

