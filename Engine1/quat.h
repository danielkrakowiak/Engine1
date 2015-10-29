#pragma once

class float3;
class float33;
class float43;
class float44;

class quat
{
	public:

	float w, x, y, z;

	// Can be used to find "similarity" between two quaternions. Equals to the cosine of the angular difference between two quaternions.
	static float dot( const quat& a, const quat& b );
	
	// Spherical linear interpolation between two quaternions.
	static quat slerp( const quat& quatFrom, const quat& quatTo, float t );

	// Return a quaternion representing the opposite angular displacement of the input quaternion.
	static quat conjugate( const quat& q );
	
	// Quaternion exponentiation. Allows extracting "fraction" of an angular displacement from a quaternion.
	static quat pow( const quat& q, float exponent );

	quat() {}

	quat( float w, float x, float y, float z )
	{
		this->w = w;
		this->x = x;
		this->y = y;
		this->z = z;
	}

	quat( const float33& m )
	{
		set( m );
	}

	quat( const float43& m )
	{
		set( m );
	}

	quat( const float44& m )
	{
		set( m );
	}

	~quat() {}

	void identity()
	{
		w = 1.0f;
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}

	// Useful to remove numerical errors after many operations have been performed on the quaternion. 
	void normalize();

	// Quaternion cross product, which concatenates multiple angular displacements. The order of multiplication, 
	// from left to right, corresponds to the order that the angular displacements are applied. 
	// This is backwards from the "standard" definition of quaternion multiplication (for ease of use).
	quat operator * ( const quat& q ) const;

	// Quaternion cross product, which concatenates two angular displacements.
	quat& operator *= ( const quat& q );

	void set( const float33& m );
	void set( const float43& m );
	void set( const float44& m );

	float getRotationAngle() const;

	float3 getRotationAxis() const;
};

