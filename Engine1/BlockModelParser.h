#pragma once

#include <string>
#include <vector>
#include <memory>

struct ID3D11Device;

namespace Engine1
{
    class BlockModel;

    class BlockModelParser
    {
        friend class BlockModel;

        private:
        static std::shared_ptr<BlockModel> parseBinary( std::vector<char>::const_iterator& dataIt, const bool loadRecurrently, ID3D11Device& device );
        static void                        writeBinary( std::vector<char>& data, const BlockModel& model );
    };
}

