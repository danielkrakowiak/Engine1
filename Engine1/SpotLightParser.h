#pragma once

#include <vector>
#include <memory>

namespace Engine1
{
    class SpotLight;

    class SpotLightParser
    {
        friend class SpotLight;

        private:
        static std::shared_ptr< SpotLight > parseBinary( std::vector< char >::const_iterator& dataIt );
        static void                         writeBinary( std::vector< char >& data, const SpotLight& light );
    };
};



