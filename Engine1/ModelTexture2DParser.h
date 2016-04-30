#pragma once

#include <vector>
#include <memory>
#include <d3d11.h>

namespace Engine1
{
    class ModelTexture2D;

    class ModelTexture2DParser
    {
        friend class ModelTexture2D;

        private:
        static std::shared_ptr<ModelTexture2D> parseBinary( std::vector<char>::const_iterator& dataIt, const bool loadRecurrently, ID3D11Device& device );
        static void                            writeBinary( std::vector<char>& data, const ModelTexture2D& modelTexture );
    };
}

