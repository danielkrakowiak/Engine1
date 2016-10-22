#pragma once

#include "MathUtil.h"

#include <algorithm>

#include "BlockActor.h"
#include "BlockModel.h"

using namespace Engine1;

float MathUtil::radiansToDegrees( float radians ) {
	return ( radians / pi ) * 180.0f;
}

float MathUtil::degreesToRadians( float degrees ) {
	return ( degrees / 180.0f ) * pi;
}

bool MathUtil::areEqual( float A, float B, float maxRelDiff, float maxAbsDiff ) {
	// Check if the numbers are really close -- needed
	// when comparing numbers near zero.
	float diff = fabs( A - B );
	if ( diff <= maxAbsDiff ) return true;

	A = fabs( A );
	B = fabs( B );
	float largest = ( B > A ) ? B : A;

	if ( diff <= largest * maxRelDiff )	return true;

	return false;
}

bool MathUtil::areEqual( double A, double B, double maxRelDiff, double maxAbsDiff ) {
	// Check if the numbers are really close -- needed
	// when comparing numbers near zero.
	double diff = fabs( A - B );
	if ( diff <= maxAbsDiff ) return true;

	A = fabs( A );
	B = fabs( B );
	double largest = ( B > A ) ? B : A;

	if ( diff <= largest * maxRelDiff )	return true;

	return false;
}

bool MathUtil::areEqual( float2 A, float2 B, float maxRelDiff, float maxAbsDiff ) {
	return areEqual( A.x, B.x, maxAbsDiff, maxRelDiff )
		&& areEqual( A.y, B.y, maxAbsDiff, maxRelDiff );
}

bool MathUtil::areEqual( float3 A, float3 B, float maxRelDiff, float maxAbsDiff ) {
	return areEqual( A.x, B.x, maxAbsDiff, maxRelDiff )
		&& areEqual( A.y, B.y, maxAbsDiff, maxRelDiff )
		&& areEqual( A.z, B.z, maxAbsDiff, maxRelDiff );
}

bool MathUtil::areEqual( float4 A, float4 B, float maxRelDiff, float maxAbsDiff )
{
	return areEqual( A.x, B.x, maxAbsDiff, maxRelDiff )
		&& areEqual( A.y, B.y, maxAbsDiff, maxRelDiff )
		&& areEqual( A.z, B.z, maxAbsDiff, maxRelDiff )
		&& areEqual( A.w, B.w, maxAbsDiff, maxRelDiff );
}

bool MathUtil::areEqual( quat A, quat B, float maxRelDiff, float maxAbsDiff )
{
	return areEqual( A.w, B.w, maxAbsDiff, maxRelDiff )
		&& areEqual( A.x, B.x, maxAbsDiff, maxRelDiff )
		&& areEqual( A.y, B.y, maxAbsDiff, maxRelDiff )
		&& areEqual( A.z, B.z, maxAbsDiff, maxRelDiff );
}

float44 MathUtil::lookAtTransformation( float3 at, float3 eye, float3 up ) {
	float3 zaxis = at - eye;
	zaxis.normalize( );
	float3 xaxis = cross( up, zaxis );
	xaxis.normalize( );
	float3 yaxis = cross( zaxis, xaxis );

	return float44(
		xaxis.x, yaxis.x, zaxis.x, 0.0f,
		xaxis.y, yaxis.y, zaxis.y, 0.0f,
		xaxis.z, yaxis.z, zaxis.z, 0.0f,
		-dot( xaxis, eye ), -dot( yaxis, eye ), -dot( zaxis, eye ), 1.0f
		);

}

float44 MathUtil::perspectiveProjectionTransformation( float fovY, float aspect, float zNear, float zFar ) {
	float yScale = cos( fovY / 2.0f ) / sin( fovY / 2.0f ); //ctg( fovY / 2 )
	float xScale = yScale / aspect;

	return float44(
		xScale, 0.0f, 0.0f, 0.0f,
		0.0f, yScale, 0.0f, 0.0f,
		0.0f, 0.0f, zFar / ( zFar - zNear ), 1.0f,
		0.0f, 0.0f, -zNear*zFar / ( zFar - zNear ), 0.0f
		);
}

float44 MathUtil::orthographicProjectionTransformation( float width, float height, float zNear, float zFar ) {
	return float44(
		2.0f / width, 0.0f, 0.0f, 0.0f,
		0.0f, 2.0f / height, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f / ( zFar - zNear ), 0.0f,
		0.0f, 0.0f, zNear / ( zNear - zFar ), 1.0f
		);
}

float33 MathUtil::anglesToRotationMatrix( const float3& rot ) {
	//source: www.gamedev.net
	float a = cos( rot.x );
	float b = sin( rot.x );
	float c = cos( rot.y );
	float d = sin( rot.y );
	float e = cos( rot.z );
	float f = sin( rot.z );
	float ad = a*d;
	float bd = b*d;

	return float33(
		c*e,
		-c*f,
		d,
		bd*e + a*f,
		-bd*f + a*e,
		-b*c,
		-ad*e + b*f,
		ad*f + b*e,
		a*c
		);
}

float3 MathUtil::rotationMatrixToAngles( const float33& mat ) {
	//source: www.gamedev.net
	float3 rot;
	rot.y = asin( mat.m13 ); //Calculate Y-axis angle
	float C = cos( rot.y );

	if ( fabs( C ) > 0.005 ) //Gimball lock?
	{
		float trX =  mat.m33 / C;  //No, so get X-axis angle
		float trY = -mat.m23 / C;
		rot.x = atan2( trY, trX );
		trX =  mat.m11 / C;        //Get Z-axis angle
		trY = -mat.m12 / C;
		rot.z = atan2( trY, trX );
	} else                         // Gimball lock has occurred
	{
		rot.x = 0;                 // Set X-axis angle to zero
		float trX = mat.m22;       // And calculate Z-axis angle
		float trY = mat.m21;
		rot.z = atan2( trY, trX );;
	}

	// return only positive angles in [0, 2Pi]
	if ( rot.x < 0 ) rot.x += piTwo;
	if ( rot.y < 0 ) rot.y += piTwo;
	if ( rot.z < 0 ) rot.z += piTwo;

	return rot;
}

std::tuple<float3, float3> MathUtil::calculateBoundingBox( const std::vector<float3>& vertices )
{
    // Note: Could be optimized if needed.

    if ( vertices.empty() )
        return std::make_tuple( float3::ZERO, float3::ZERO );

    float3 bbMin(  FLT_MAX,  FLT_MAX,  FLT_MAX );
    float3 bbMax( -FLT_MAX, -FLT_MAX, -FLT_MAX );

    for ( const float3& vertex : vertices )
    {
        bbMin = min( bbMin, vertex );
        bbMax = max( bbMax, vertex );
    }

    return std::make_tuple( bbMin, bbMax );
}

float3 MathUtil::getRayDirectionAtPixel( const float43& cameraPose, const float2& targetPixel,
                                         const float2& screenDimensions, const float fieldOfView )
{
    // viewportCenter - viewport plane center in world space.
    // viewportUp - viewport up vector in world space. It's length equals half of height of the viewport plane.
    // viewportRight - viewport right vector in world space. It's length equals half of width of the viewport plane.
    // screenDimensionsHalf - viewport dimensions in pixels divided by 2.
    const float  screenAspect          = screenDimensions.x / screenDimensions.y;
    const float2 screenDimensionsHalf  = screenDimensions * 0.5f;
    const float3 viewportUp            = cameraPose.getRow2() * tan( fieldOfView * 0.5f );
    const float3 viewportRight         = cameraPose.getRow1() * screenAspect * viewportUp.length();
    const float3 viewportCenter        = cameraPose.getTranslation() + cameraPose.getRow3();

    const float2 pixelShift = (targetPixel - screenDimensionsHalf + float2(0.5f, 0.5f)) / screenDimensionsHalf; // In range (-1;1)

	const float3 pixelPosWorld = viewportCenter + viewportRight * pixelShift.x - viewportUp * pixelShift.y;
	
    float3 rayDir = pixelPosWorld - cameraPose.getTranslation();
    rayDir.normalize();

    return rayDir;
}

std::tuple< bool, float > MathUtil::intersectRayWithBoundingBox( const float3& rayOriginWorld, const float3& rayDirWorld, const float43& boxPose, const float3& boxMinLocal, const float3& boxMaxLocal )
{
    const float43 boxPoseInverse = boxPose.getScaleOrientationTranslationInverse(); 

    float3 rayOriginLocal = rayOriginWorld * boxPoseInverse;
    float3 rayDirLocal    = ( (rayOriginWorld + rayDirWorld) * boxPoseInverse ) - rayOriginLocal;

    // Transform ray to box's local space and check for intersection.
    return intersectRayWithBoundingBox( rayOriginLocal, rayDirLocal, boxMinLocal, boxMaxLocal );
}

std::tuple< bool, float > MathUtil::intersectRayWithBoundingBox( const float3& rayOriginInBoxSpace, const float3& rayDirInBoxSpace, const float3& boxMinLocal, const float3& boxMaxLocal )
{
    float tmin = -FLT_MAX;
	float tmax =  FLT_MAX;
 
	const float3 t1 = ( boxMinLocal - rayOriginInBoxSpace ) / rayDirInBoxSpace;
	const float3 t2 = ( boxMaxLocal - rayOriginInBoxSpace ) / rayDirInBoxSpace;
 
	tmin = std::max(tmin, std::min(t1.x, t2.x));
	tmax = std::min(tmax, std::max(t1.x, t2.x));
	tmin = std::max(tmin, std::min(t1.y, t2.y));
	tmax = std::min(tmax, std::max(t1.y, t2.y));
	tmin = std::max(tmin, std::min(t1.z, t2.z));
	tmax = std::min(tmax, std::max(t1.z, t2.z));

    const bool hit = tmax >= tmin && tmax > 0.0f;

    if ( hit )
        return std::make_tuple( true, tmin );
    else
        return std::make_tuple( false, 0.0f );
}

std::tuple< bool, float > MathUtil::intersectRayWithBlockActor( const float3& rayOriginWorld, const float3& rayDirWorld, const BlockActor& actor, const float maxDist )
{

    if ( !actor.getModel() || !actor.getModel()->getMesh() )
        return std::make_tuple( false, 0.0f );

    const float43&   worldToLocalMatrix = actor.getPose().getScaleOrientationTranslationInverse();
    const BlockMesh& mesh               = *actor.getModel()->getMesh();

    const float3 rayOriginLocal = rayOriginWorld * worldToLocalMatrix;
    const float3 rayDirLocal    = ((rayOriginWorld + rayDirWorld) * worldToLocalMatrix) - rayOriginLocal;

    float3 bbMin, bbMax;
    std::tie( bbMin, bbMax ) = mesh.getBoundingBox();
    
    bool  bbHit         = false;
    float bbHitDistance = FLT_MAX;;
    std::tie( bbHit, bbHitDistance ) = intersectRayWithBoundingBox( rayOriginLocal, rayDirLocal, bbMin, bbMax );

    if ( !bbHit || bbHitDistance > maxDist )
        return std::make_tuple( false, 0.0f );

    bool  hit         = false;
    float hitDistance = FLT_MAX;

    for ( const uint3& triangle : mesh.getTriangles() )
    {
        const float3& vertexPos1 = mesh.getVertices()[ triangle.x ];
        const float3& vertexPos2 = mesh.getVertices()[ triangle.y ];
        const float3& vertexPos3 = mesh.getVertices()[ triangle.z ];

        if ( rayTriangleIntersect( rayOriginLocal, rayDirLocal, vertexPos1, vertexPos2, vertexPos3 ) )
        {
            const float dist = calcDistToTriangle( rayOriginLocal, rayDirLocal, vertexPos1, vertexPos2, vertexPos3 );
            if ( dist > 0.0f )
            {
                hit         = true;
                hitDistance = std::min( hitDistance, dist );
            }
        }
    }

    return std::make_tuple( hit, hitDistance );
}

bool MathUtil::rayTriangleIntersect( const float3& rayOrigin, const float3& rayDir, 
                                     const float3& vertexPos1, const float3& vertexPos2, const float3& vertexPos3 )
{
    const float dot1 = dot( rayDir, cross( vertexPos1 - rayOrigin, vertexPos2 - rayOrigin ) );
    const float dot2 = dot( rayDir, cross( vertexPos2 - rayOrigin, vertexPos3 - rayOrigin ) );
    const float dot3 = dot( rayDir, cross( vertexPos3 - rayOrigin, vertexPos1 - rayOrigin ) );

    // Without backface culling:
    //return ( ( dot1 < 0 && dot2 < 0 && dot3 < 0 ) || ( dot1 > 0 && dot2 > 0 && dot3 > 0 ) );

    // With backface culling:
    return (dot1 < 0 && dot2 < 0 && dot3 < 0);
}

float MathUtil::calcDistToTriangle( const float3& rayOrigin, const float3& rayDir, 
                                    const float3& vertexPos1, const float3& vertexPos2, const float3& vertexPos3 )
{
    float3 trianglePlaneNormal = ( cross( vertexPos2 - vertexPos1, vertexPos3 - vertexPos1 ) );
    trianglePlaneNormal.normalize();

    const float  trianglePlaneDistance = -dot( vertexPos1, trianglePlaneNormal );

    const float rayTriangleDot = dot( rayDir, trianglePlaneNormal );
    const float distFromRayOrigin = -( dot( trianglePlaneNormal, rayOrigin ) + trianglePlaneDistance ) / rayTriangleDot;

    return distFromRayOrigin;
}

bool MathUtil::isPowerOfTwo( unsigned int number )
{
    return number != 0 && !( number & ( number - 1 ) );
}