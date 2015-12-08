#pragma once

#include "Light.h"

#include <vector>
#include <memory>

using namespace Engine1;

namespace Engine1
{
    class PointLight : public Light
    {
        public:

        static std::shared_ptr<PointLight> createFromFile( const std::string& path );
        static std::shared_ptr<PointLight> createFromMemory( std::vector<char>::const_iterator& dataIt );

        PointLight();
        PointLight( const float3& position, const float3& diffuseColor = float3::ZERO, const float3& specularColor = float3::ZERO );
        ~PointLight();

        void saveToMemory( std::vector<char>& data );
        void saveToFile( const std::string& path );

        virtual Type getType( ) const;
    };
};

