#pragma once

#include "float3.h"

class Camera {
public:
	Camera();
	~Camera();

	void setPosition( float3 position ) { this->position = position; }
	void setDirection( float3 direction ) { this->direction = direction; }
	void setUp( float3 up ) { this->up = up; }
	void setFieldOfView( float fieldOfView ) { this->fieldOfView = fieldOfView; }

	float3 getPosition() { return position; }
	float3 getDirection() { return direction; }
	float3 getUp() { return up; }
	float3 getLookAtPoint( ) { return position + direction; }
	float getFieldOfView() { return fieldOfView; }
	

protected:
	float3 position;
	float3 direction;
	float3 up;
	//in radians
	float fieldOfView;
};

