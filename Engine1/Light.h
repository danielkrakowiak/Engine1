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

        void setEnabled( const bool enabled );
        void setPosition( const float3& position );
        void setColor( const float3& color );

        bool   isEnabled() const;
        float3 getPosition() const;
        float3 getColor( ) const;

        protected:

        Light();
        Light( const float3& position, const float3& color, const bool enabled = true );
        ~Light();

        bool   m_enabled;
        float3 m_position;
        float3 m_color;
    };
};

