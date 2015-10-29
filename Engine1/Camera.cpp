#include "Camera.h"

#include "MathUtil.h"


Camera::Camera( ) : position( 0.0f, 0.0f, 0.0f ), direction( 0.0f, 0.0f, 1.0f ), up( 0.0f, 1.0f, 0.0f ), fieldOfView( MathUtil::piHalf ) {}


Camera::~Camera() {}


