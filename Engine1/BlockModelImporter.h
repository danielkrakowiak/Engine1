#pragma once

#include <string>
#include <vector>
#include <memory>

#include "Texture2D.h"

struct ID3D11Device3;

namespace Engine1
{
    class BlockModel;

    class BlockModelImporter
    {
        public:

        // Note: This method loads mesh and materials from the file, but doesn't load textures.
        // Should be used only to convert to internal file format.
        static std::vector< std::shared_ptr< BlockModel > > import( 
            std::string filePath, 
            const bool invertZCoordinate, 
            const bool invertVertexWindingOrder, 
            const bool flipUVs
        );

        private:

        static std::shared_ptr< Texture2D< TexUsage::Default, TexBind::ShaderResource, uchar4 > > createDefaultWhiteTexture();
    };
}

