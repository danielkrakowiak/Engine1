#pragma once

#include "float3.h"

namespace Engine1
{
    class Camera
    {
        public:
        Camera();
        ~Camera();

        void setPosition( float3 position );
        void setDirection( float3 direction );
        void setUp( float3 up );
        void setFieldOfView( float fieldOfView );

        float3 getPosition() const;
        float3 getDirection() const;
        float3 getUp() const;
        float3 getRight() const;
        float3 getLookAtPoint() const;
        float  getFieldOfView() const;

        protected:
        float3 position;
        float3 direction;
        float3 up;
        // In radians.
        float fieldOfView;
    };
}

