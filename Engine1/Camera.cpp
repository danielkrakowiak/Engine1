#include "Camera.h"

#include "MathUtil.h"

using namespace Engine1;

Camera::Camera() :
m_position( float3::ZERO ),
m_direction( 0.0f, 0.0f, 1.0f ),
m_up( 0.0f, 1.0f, 0.0f ),
m_fieldOfView( MathUtil::piHalf )
{}

Camera::~Camera()
{}

void Camera::setPosition( float3 position )
{
    this->m_position = position;
}
void Camera::setDirection( float3 direction )
{
    this->m_direction = direction;
}

void Camera::setUp( float3 up ) 
{
    this->m_up = up; 
}

void Camera::setFieldOfView( float fieldOfView ) 
{ 
    this->m_fieldOfView = fieldOfView; 
}

float3 Camera::getPosition() const 
{ 
    return m_position; 
}

float3 Camera::getDirection() const 
{ 
    return m_direction; 
}

float3 Camera::getUp() const 
{ 
    return m_up; 
}

float3 Camera::getRight() const
{
    return cross(m_up, m_direction);
}

float3 Camera::getLookAtPoint() const 
{ 
    return m_position + m_direction; 
}

float Camera::getFieldOfView() const 
{ 
    return m_fieldOfView; 
}


