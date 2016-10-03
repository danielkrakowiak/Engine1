#pragma once

#include "Light.h"

#include <vector>
#include <memory>

namespace Engine1
{
    class PointLight : public Light
    {
        public:

        static std::shared_ptr<PointLight> createFromFile( const std::string& path );
        static std::shared_ptr<PointLight> createFromMemory( std::vector<char>::const_iterator& dataIt );

        PointLight();
        PointLight( const float3& position, const float3& color = float3( 1.0f, 1.0f, 1.0f ) );
        ~PointLight();

        void saveToMemory( std::vector<char>& data );
        void saveToFile( const std::string& path );

        virtual Type getType( ) const;
    };
};

