#include "Camera.h"

#include "MathUtil.h"

using namespace Engine1;

Camera::Camera() :
position( float3::ZERO ),
direction( 0.0f, 0.0f, 1.0f ),
up( 0.0f, 1.0f, 0.0f ),
fieldOfView( MathUtil::piHalf )
{}

Camera::~Camera()
{}

void Camera::setPosition( float3 position )
{
    this->position = position;
}
void Camera::setDirection( float3 direction )
{
    this->direction = direction;
}

void Camera::setUp( float3 up ) 
{
    this->up = up; 
}

void Camera::setFieldOfView( float fieldOfView ) 
{ 
    this->fieldOfView = fieldOfView; 
}

float3 Camera::getPosition() const 
{ 
    return position; 
}

float3 Camera::getDirection() const 
{ 
    return direction; 
}

float3 Camera::getUp() const 
{ 
    return up; 
}

float3 Camera::getRight() const
{
    return cross(up, direction);
}

float3 Camera::getLookAtPoint() const 
{ 
    return position + direction; 
}

float Camera::getFieldOfView() const 
{ 
    return fieldOfView; 
}


