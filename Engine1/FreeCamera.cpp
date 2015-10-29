#include "FreeCamera.h"

#include "MathUtil.h"

const float FreeCamera::defaultSpeedDamping = 40.0f; //per second
const float FreeCamera::defaultSpeedAcceleration = 15.0f; //per second
const float FreeCamera::defaultMaxSpeed = 100.0f; //per second

FreeCamera::FreeCamera( ) : 
rotationAngles( 0.0f, 0.0f, 0.0f ),
forwardSpeed( 0.0f ),
sideSpeed( 0.0f ),
upSpeed( 0.0f ),
speedDamping( defaultSpeedDamping ),
speedAcceleration( defaultSpeedAcceleration ),
maxSpeed( defaultMaxSpeed ),
dampSpeedForward( false ),
dampSpeedSide( false ), 
dampSpeedUp( false ) {
}


FreeCamera::~FreeCamera() {}

//timeLapse in ms
void FreeCamera::accelerateForward( float timeLapse ) {
	timeLapse /= 1000.0f;
	dampSpeedForward = ( forwardSpeed < 0.0f ); //don't damp speed if the camera is moving forward

	forwardSpeed += speedAcceleration * timeLapse;
	if ( forwardSpeed > maxSpeed ) forwardSpeed = maxSpeed;
}

//timeLapse in ms
void FreeCamera::accelerateReverse( float timeLapse ) {
	timeLapse /= 1000.0f;
	dampSpeedForward = ( forwardSpeed > 0.0f ); //don't damp speed if the camera is moving backward

	forwardSpeed -= speedAcceleration * timeLapse;
	if ( forwardSpeed < -maxSpeed ) forwardSpeed = -maxSpeed;
}

//timeLapse in ms
void FreeCamera::accelerateRight( float timeLapse ) {
	timeLapse /= 1000.0f;
	dampSpeedSide = ( sideSpeed < 0.0f ); //don't damp speed if the camera is moving right

	sideSpeed += speedAcceleration * timeLapse;
	if ( sideSpeed > maxSpeed ) sideSpeed = maxSpeed;
}

//timeLapse in ms
void FreeCamera::accelerateLeft( float timeLapse ) {
	timeLapse /= 1000.0f;
	dampSpeedSide = ( sideSpeed > 0.0f ); //don't damp speed if the camera is moving left

	sideSpeed -= speedAcceleration * timeLapse;
	if ( sideSpeed < -maxSpeed ) sideSpeed = -maxSpeed;
}

//timeLapse in ms
void FreeCamera::accelerateUp( float timeLapse ) {
	timeLapse /= 1000.0f;
	dampSpeedUp = ( upSpeed < 0.0f ); //don't damp speed if the camera is moving up

	upSpeed += speedAcceleration * timeLapse;
	if ( upSpeed > maxSpeed ) upSpeed = maxSpeed;
}

//timeLapse in ms
void FreeCamera::accelerateDown( float timeLapse ) {
	timeLapse /= 1000.0f;
	dampSpeedUp = ( upSpeed > 0.0f ); //don't damp speed if the camera is moving down

	upSpeed -= speedAcceleration * timeLapse;
	if ( upSpeed < -maxSpeed ) upSpeed = -maxSpeed;
}

//timeLapse in ms
void FreeCamera::updateState( float timeLapse ) {
	timeLapse /= 1000.0f;
	float frameSpeedDamping = speedDamping * timeLapse;

	if ( dampSpeedForward ) {
		if ( forwardSpeed >= frameSpeedDamping ) forwardSpeed -= frameSpeedDamping;
		else if ( forwardSpeed <= -frameSpeedDamping ) forwardSpeed += frameSpeedDamping;
		else forwardSpeed = 0.0f;
	}

	if ( dampSpeedSide ) {
		if ( sideSpeed >= frameSpeedDamping ) sideSpeed -= frameSpeedDamping;
		else if ( sideSpeed <= -frameSpeedDamping ) sideSpeed += frameSpeedDamping;
		else sideSpeed = 0.0f;
	}

	if ( dampSpeedUp ) {
		if ( upSpeed >= frameSpeedDamping ) upSpeed -= frameSpeedDamping;
		else if ( upSpeed <= -frameSpeedDamping ) upSpeed += frameSpeedDamping;
		else upSpeed = 0.0f;
	}

	move( float3( sideSpeed, upSpeed, forwardSpeed ) * timeLapse );

	dampSpeedForward = true;
	dampSpeedSide = true;
	dampSpeedUp = true;
}

void FreeCamera::setDirection( float3 direction ) {
	this->direction = direction;

	updateRotationAnglesFromOrientation();
}

void FreeCamera::setUp( float3 up ) {
	this->up = up;

	updateRotationAnglesFromOrientation( );
}

void FreeCamera::updateRotationAnglesFromOrientation( ) {
	//try to deduce angles from camera orientation
	float3 sideDirection = float3::cross( up, direction );
	rotationAngles = MathUtil::rotationMatrixToAngles( float33( sideDirection, up, direction ) );
}

float3 FreeCamera::getSpeed() {
	return float3( sideSpeed, upSpeed, forwardSpeed );
}

void FreeCamera::move( float3 move ) {
	float3 sideDirection = float3::cross( up, direction );
	position += move.x * sideDirection;
	position += move.y * up;
	position += move.z * direction;
}

void FreeCamera::rotate( float3 rotationAnglesChange ) {
	rotationAngles += rotationAnglesChange;

	
	float33 rotationTransformation = MathUtil::anglesToRotationMatrix( rotationAngles );
	direction = float3( 0.0f, 0.0f, 1.0f ) * rotationTransformation;
	up = float3( 0.0f, 1.0f, 0.0f ) * rotationTransformation;
}
