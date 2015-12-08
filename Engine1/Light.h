#pragma once

#include "float3.h"

namespace Engine1
{
    class Light
    {
        public:

        enum class Type : char
        {
            PointLight = 0
        };

        virtual Type getType() const = 0;

        void setPosition( const float3& position );
        void setDiffuseColor( const float3& diffuseColor );
        void setSpecularColor( const float3& specularColor );

        float3 getPosition() const;
        float3 getDiffuseColor( ) const;
        float3 getSpecularColor( ) const;

        protected:

        Light();
        Light( const float3& position, const float3& diffuseColor, const float3& specularColor );
        ~Light();

        float3 position;

        float3 diffuseColor;
        float3 specularColor;
    };
};

