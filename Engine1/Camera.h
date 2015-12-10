#pragma once

#include "float3.h"

namespace Engine1
{
    class Camera
    {
        public:
        Camera();
        ~Camera();

        void setPosition( float3 position ) { this->position = position; }
        void setDirection( float3 direction ) { this->direction = direction; }
        void setUp( float3 up ) { this->up = up; }
        void setFieldOfView( float fieldOfView ) { this->fieldOfView = fieldOfView; }

        float3 getPosition() const { return position; }
        float3 getDirection() const { return direction; }
        float3 getUp() const { return up; }
        float3 getLookAtPoint() const { return position + direction; }
        float getFieldOfView() const { return fieldOfView; }


        protected:
        float3 position;
        float3 direction;
        float3 up;
        //in radians
        float fieldOfView;
    };
}

