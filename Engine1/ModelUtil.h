#pragma once

#include <memory>
#include <vector>

struct ID3D11Device3;

namespace Engine1
{
    class BlockModel;

    class ModelUtil
    {
        public:

        static std::shared_ptr< BlockModel > mergeModels( const std::vector< std::shared_ptr< BlockModel > >& models, ID3D11Device3& device );

        private: 

        static std::string getDescription( const BlockModel& model, const bool printPath = false, const bool printDimensions = false );
    };
};

