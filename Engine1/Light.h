#pragma once

#include "float3.h"

namespace Engine1
{
    class Light
    {
        public:

        enum class Type : char
        {
            PointLight = 0,
            SpotLight
        };

        virtual Type getType() const = 0;

        void setEnabled( const bool enabled );
        void setCastingShadows( const bool castingShadows );
        void setPosition( const float3& position );
        void setColor( const float3& color );
        void setEmitterRadius( const float emitterRadius );

        bool   isEnabled() const;
        bool   isCastingShadows() const;
        float3 getPosition() const;
        float3 getColor( ) const;
        float  getEmitterRadius() const;

        protected:

        Light();
        Light( const float3& position, const float3& color, const bool enabled = true, const bool castingShadows = true, const float emitterRadius = 0.0f );
        ~Light();

        bool   m_enabled;
        bool   m_castingShadows;
        float3 m_position;
        float3 m_color;
        float  m_emitterRadius; // Note: radius of 0 means point light, radius > 0 means an area light of given radius.
    };
};

