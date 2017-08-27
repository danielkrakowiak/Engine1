#pragma once

#include <memory>
#include <vector>
#include <array>

#include "MathUtil.h"
#include "Model.h"

struct ID3D11Device3;

namespace Engine1
{
    class BlockModel;

    class ModelUtil
    {
        public:

        static std::shared_ptr< BlockModel > mergeModels( 
            const std::vector< std::shared_ptr< BlockModel > >& models, 
            const std::vector< float43 >& transforms, 
            ID3D11Device3& device 
        );

        private: 

        static std::string getDescription( 
            const BlockModel& model, 
            const bool printPath = false, 
            const bool printDimensions = false 
        );

        struct TextureSet
        {
            TextureSet();

            bool operator == (TextureSet& other) const;

            void calculateDimensions();
            int2 getTextureDimensions( Model::TextureType textureType ) const;
               
            // Dimensions required to store the set without loosing resolution for any type of texture.
            int2 m_dimensions;

            std::array< std::shared_ptr< Asset >, (int)Model::TextureType::COUNT > m_textures;
            std::array< float4, (int)Model::TextureType::COUNT >                   m_colorMultipliers;
        };
    };
};

