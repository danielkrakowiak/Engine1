#pragma once

#include <string>
#include <vector>
#include <memory>

struct ID3D11Device;

namespace Engine1
{
    class FreeCamera;

    class FreeCameraParser
    {
        friend class FreeCamera;

        private:

        static std::shared_ptr< FreeCamera > parseBinary( std::vector< char >::const_iterator& dataIt );
        static void                          writeBinary( std::vector< char >& data, const FreeCamera& camera );
    };
}

