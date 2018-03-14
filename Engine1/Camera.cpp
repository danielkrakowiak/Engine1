#include "Camera.h"

#include "MathUtil.h"
#include "Settings.h"

using namespace Engine1;

Camera::Camera() :
    m_position( float3::ZERO ),
    m_direction( 0.0f, 0.0f, 1.0f ),
    m_up( 0.0f, 1.0f, 0.0f ),
    m_fieldOfView( MathUtil::degreesToRadians( settings().rendering.fieldOfViewDegress ) )
{}

Camera::Camera( float3 position, float3 direction, float3 up, float fieldOfView ) :
    m_position( position ),
    m_direction( direction ),
    m_up( up ),
    m_fieldOfView( fieldOfView )
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

void Camera::setInterpolated( const Camera& camera1, const Camera& camera2, float ratio )
{
    ratio = std::min( 1.0f, std::max( 0.0f, ratio ) );

    float33 orientation1;
    orientation1.setRow1( cross(camera1.m_up, camera1.m_direction) );
    orientation1.setRow2(camera1.m_up);
    orientation1.setRow3(camera1.m_direction);

    float33 orientation2;
    orientation2.setRow1( cross(camera2.m_up, camera2.m_direction) );
    orientation2.setRow2(camera2.m_up);
    orientation2.setRow3(camera2.m_direction);

    auto slerpedOrientation = float33::slerp( orientation1, orientation2, ratio );

    m_position    = MathUtil::lerp( camera1.m_position, camera2.m_position, ratio );
    m_direction   = slerpedOrientation.getRow3();
    m_up          = slerpedOrientation.getRow2();
    m_fieldOfView = MathUtil::lerp( camera1.m_fieldOfView, camera2.m_fieldOfView, ratio );
}


