#pragma once

#include "Camera.h"

#include <memory>
#include <string>
#include <vector>

namespace Engine1
{
    class FreeCamera : public Camera
    {
        friend class FreeCameraParser;

        public:

        static std::shared_ptr< FreeCamera > createFromFile( const std::string& path );
        static std::shared_ptr< FreeCamera > createFromMemory( std::vector< char >::const_iterator& dataIt, const std::vector< char >::const_iterator& dataEndIt );

        FreeCamera();
        FreeCamera( float3 position, float3 rotationAngles, float fieldOfView );
        ~FreeCamera();

        void saveToMemory( std::vector< char >& data ) const;
        void saveToFile( const std::string& path ) const;

        void setDirection( float3 direction );
        void setUp( float3 up );

        void updateState( float timeLapse );

        void accelerateForward( float timeLapse );
        void accelerateReverse( float timeLapse );
        void accelerateRight( float timeLapse );
        void accelerateLeft( float timeLapse );
        void accelerateUp( float timeLapse );
        void accelerateDown( float timeLapse );

        float3 getAcceleration();
        float3 getSpeed();

        // side move, up move, forward move
        void move( float3 move );
        // around side axis, up axis, forward axis
        void rotate( float3 rotationAngles );

        private:

        void updateRotationAnglesFromOrientation();
        void updateOrientationFromRotationAngles();

        float3 m_rotationAngles;

        bool m_dampSpeedForward, m_dampSpeedSide, m_dampSpeedUp;
        float m_forwardSpeed;
        float m_sideSpeed;
        float m_upSpeed;
        float m_speedDamping;
        float m_speedAcceleration;
        float m_maxSpeed;

        static const float m_defaultSpeedDamping;
        static const float m_defaultSpeedAcceleration;
        static const float m_defaultMaxSpeed;
    };
}

