#pragma once
#include "Camera.h"

class FreeCamera : public Camera {
public:
	FreeCamera();
	~FreeCamera();

	void setDirection( float3 direction );
	void setUp( float3 up );

	void updateState( float timeLapse );

	void accelerateForward( float timeLapse );
	void accelerateReverse( float timeLapse );
	void accelerateRight( float timeLapse );
	void accelerateLeft( float timeLapse );
	void accelerateUp( float timeLapse );
	void accelerateDown( float timeLapse );

	float3 getAcceleration( );
	float3 getSpeed();

	// side move, up move, forward move
	void move( float3 move );
	// around side axis, up axis, forward axis
	void rotate( float3 rotationAngles );

private:

	void updateRotationAnglesFromOrientation();
	float3 rotationAngles;

	bool dampSpeedForward, dampSpeedSide, dampSpeedUp;
	float forwardSpeed;
	float sideSpeed;
	float upSpeed;
	float speedDamping;
	float speedAcceleration;
	float maxSpeed;

	static const float defaultSpeedDamping;
	static const float defaultSpeedAcceleration;
	static const float defaultMaxSpeed;
};

