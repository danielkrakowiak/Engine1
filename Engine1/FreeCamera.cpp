#include "FreeCamera.h"

#include "MathUtil.h"

#include "BinaryFile.h"
#include "FreeCameraParser.h"

using namespace Engine1;

const float FreeCamera::m_defaultSpeedDamping = 40.0f; //per second
const float FreeCamera::m_defaultSpeedAcceleration = 15.0f; //per second
const float FreeCamera::m_defaultMaxSpeed = 100.0f; //per second

std::shared_ptr< FreeCamera > FreeCamera::createFromFile( const std::string& path )
{
    std::shared_ptr< std::vector< char > > fileData = BinaryFile::load( path );

    std::shared_ptr< FreeCamera > camera = createFromMemory( fileData->cbegin(), fileData->cend() );

    return camera;
}

std::shared_ptr< FreeCamera > FreeCamera::createFromMemory( std::vector< char >::const_iterator dataIt, std::vector< char >::const_iterator dataEndIt )
{
    return FreeCameraParser::parseBinary( dataIt );
}

FreeCamera::FreeCamera( ) : 
    m_rotationAngles( 0.0f, 0.0f, 0.0f ),
    m_forwardSpeed( 0.0f ),
    m_sideSpeed( 0.0f ),
    m_upSpeed( 0.0f ),
    m_speedDamping( m_defaultSpeedDamping ),
    m_speedAcceleration( m_defaultSpeedAcceleration ),
    m_maxSpeed( m_defaultMaxSpeed ),
    m_dampSpeedForward( false ),
    m_dampSpeedSide( false ),
    m_dampSpeedUp( false )
{
}

FreeCamera::FreeCamera( float3 position, float3 rotationAngles, float fieldOfView ) :
    m_rotationAngles( rotationAngles ),
    m_forwardSpeed( 0.0f ),
    m_sideSpeed( 0.0f ),
    m_upSpeed( 0.0f ),
    m_speedDamping( m_defaultSpeedDamping ),
    m_speedAcceleration( m_defaultSpeedAcceleration ),
    m_maxSpeed( m_defaultMaxSpeed ),
    m_dampSpeedForward( false ),
    m_dampSpeedSide( false ),
    m_dampSpeedUp( false )
{
    m_position    = position;
    m_fieldOfView = fieldOfView;

    updateOrientationFromRotationAngles();
}

FreeCamera::~FreeCamera() {}

void FreeCamera::saveToMemory( std::vector<char>& data )
{
    FreeCameraParser::writeBinary( data, *this );
}

void FreeCamera::saveToFile( const std::string& path )
{
    std::vector<char> data;

    FreeCameraParser::writeBinary( data, *this );

    BinaryFile::save( path, data );
}

//timeLapse in ms
void FreeCamera::accelerateForward( float timeLapse ) {
	timeLapse /= 1000.0f;
	m_dampSpeedForward = ( m_forwardSpeed < 0.0f ); //don't damp speed if the camera is moving forward

	m_forwardSpeed += m_speedAcceleration * timeLapse;
	if ( m_forwardSpeed > m_maxSpeed ) m_forwardSpeed = m_maxSpeed;
}

//timeLapse in ms
void FreeCamera::accelerateReverse( float timeLapse ) {
	timeLapse /= 1000.0f;
	m_dampSpeedForward = ( m_forwardSpeed > 0.0f ); //don't damp speed if the camera is moving backward

	m_forwardSpeed -= m_speedAcceleration * timeLapse;
	if ( m_forwardSpeed < -m_maxSpeed ) m_forwardSpeed = -m_maxSpeed;
}

//timeLapse in ms
void FreeCamera::accelerateRight( float timeLapse ) {
	timeLapse /= 1000.0f;
	m_dampSpeedSide = ( m_sideSpeed < 0.0f ); //don't damp speed if the camera is moving right

	m_sideSpeed += m_speedAcceleration * timeLapse;
	if ( m_sideSpeed > m_maxSpeed ) m_sideSpeed = m_maxSpeed;
}

//timeLapse in ms
void FreeCamera::accelerateLeft( float timeLapse ) {
	timeLapse /= 1000.0f;
	m_dampSpeedSide = ( m_sideSpeed > 0.0f ); //don't damp speed if the camera is moving left

	m_sideSpeed -= m_speedAcceleration * timeLapse;
	if ( m_sideSpeed < -m_maxSpeed ) m_sideSpeed = -m_maxSpeed;
}

//timeLapse in ms
void FreeCamera::accelerateUp( float timeLapse ) {
	timeLapse /= 1000.0f;
	m_dampSpeedUp = ( m_upSpeed < 0.0f ); //don't damp speed if the camera is moving up

	m_upSpeed += m_speedAcceleration * timeLapse;
	if ( m_upSpeed > m_maxSpeed ) m_upSpeed = m_maxSpeed;
}

//timeLapse in ms
void FreeCamera::accelerateDown( float timeLapse ) {
	timeLapse /= 1000.0f;
	m_dampSpeedUp = ( m_upSpeed > 0.0f ); //don't damp speed if the camera is moving down

	m_upSpeed -= m_speedAcceleration * timeLapse;
	if ( m_upSpeed < -m_maxSpeed ) m_upSpeed = -m_maxSpeed;
}

//timeLapse in ms
void FreeCamera::updateState( float timeLapse ) {
	timeLapse /= 1000.0f;
	float frameSpeedDamping = m_speedDamping * timeLapse;

	if ( m_dampSpeedForward ) {
		if ( m_forwardSpeed >= frameSpeedDamping ) m_forwardSpeed -= frameSpeedDamping;
		else if ( m_forwardSpeed <= -frameSpeedDamping ) m_forwardSpeed += frameSpeedDamping;
		else m_forwardSpeed = 0.0f;
	}

	if ( m_dampSpeedSide ) {
		if ( m_sideSpeed >= frameSpeedDamping ) m_sideSpeed -= frameSpeedDamping;
		else if ( m_sideSpeed <= -frameSpeedDamping ) m_sideSpeed += frameSpeedDamping;
		else m_sideSpeed = 0.0f;
	}

	if ( m_dampSpeedUp ) {
		if ( m_upSpeed >= frameSpeedDamping ) m_upSpeed -= frameSpeedDamping;
		else if ( m_upSpeed <= -frameSpeedDamping ) m_upSpeed += frameSpeedDamping;
		else m_upSpeed = 0.0f;
	}

	move( float3( m_sideSpeed, m_upSpeed, m_forwardSpeed ) * timeLapse );

	m_dampSpeedForward = true;
	m_dampSpeedSide = true;
	m_dampSpeedUp = true;
}

void FreeCamera::setDirection( float3 direction ) 
{
	this->m_direction = direction;

	updateRotationAnglesFromOrientation();
}

void FreeCamera::setUp( float3 up ) 
{
	this->m_up = up;

	updateRotationAnglesFromOrientation( );
}

void FreeCamera::updateRotationAnglesFromOrientation() 
{
	//try to deduce angles from camera orientation
	float3 sideDirection = cross( m_up, m_direction );
	m_rotationAngles = MathUtil::rotationMatrixToAngles( float33( sideDirection, m_up, m_direction ) );
}

void FreeCamera::updateOrientationFromRotationAngles()
{
    float33 rotationTransformation = MathUtil::anglesToRotationMatrix( m_rotationAngles );
    m_direction = float3( 0.0f, 0.0f, 1.0f ) * rotationTransformation;
    m_up        = float3( 0.0f, 1.0f, 0.0f ) * rotationTransformation;
}

float3 FreeCamera::getSpeed() 
{
	return float3( m_sideSpeed, m_upSpeed, m_forwardSpeed );
}

void FreeCamera::move( float3 move ) 
{
	float3 sideDirection = cross( m_up, m_direction );
	m_position += move.x * sideDirection;
	m_position += move.y * m_up;
	m_position += move.z * m_direction;
}

void FreeCamera::rotate( float3 rotationAnglesChange ) 
{
	m_rotationAngles += rotationAnglesChange;

	updateOrientationFromRotationAngles();
}
