#pragma once

#include <cmath>
#undef min
#undef max

class float4;

class float3 {
public:
	float x;
	float y;
	float z;

	static float dot( const float3& vec1, const float3& vec2 );
	static float3 cross( const float3& vec1, const float3& vec2 );

	static float3 max( const float3& vec1, const float3& vec2 );
	static float3 min( const float3& vec1, const float3& vec2 );

	float3(){}

	float3(const float3& a) : x(a.x), y(a.y), z(a.z) {}

	float3(float x, float y, float z) : x(x), y(y), z(z) {}

	float3(const float4& a);

	float3& operator = (const float3& vec) {
		x = vec.x;
		y = vec.y;
		z = vec.z;

		return *this;
	}

	static int size( ) {
		return 3;
	}

	float* getData() {
		return &x;
	}

	bool operator == (const float3& vec) const {
		return x == vec.x && y == vec.y && z == vec.z;
	}

	bool operator != (const float3& vec) const {
		return x != vec.x || y != vec.y || z != vec.z;
	}

	float3 operator - () const {
		return float3(-x, -y, -z);
	}

	float3 operator + (const float3& vec) const {
		return float3(x + vec.x, y + vec.y, z + vec.z);
	}

	float3 operator - (const float3& vec) const {
		return float3(x - vec.x, y - vec.y, z - vec.z);
	}

	float3 operator * (const float value) const {
		return float3(x * value, y * value, z * value);
	}

	//Per-component multiplication
	float3 operator * (const float3& vec) const {
		return float3(x * vec.x, y * vec.y, z * vec.z);
	}

	float3 operator / (const float value) const {
		float inverse = 1.0f / value;
		return float3(x * inverse, y * inverse, z * inverse);
	}

	//Per-component division
	float3 operator / (const float3& vec) const {
		return float3(x / vec.x, y / vec.y, z / vec.z);
	}

	float3& operator += (const float value) {
		x += value;
		y += value;
		z += value;
		return *this;
	}

	float3& operator += (const float3& vec) {
		x += vec.x;
		y += vec.y;
		z += vec.z;
		return *this;
	}

	float3& operator -= (const float value) {
		x -= value;
		y -= value;
		z -= value;
		return *this;
	}

	float3& operator -= (const float3& vec) {
		x -= vec.x;
		y -= vec.y;
		z -= vec.z;
		return *this;
	}

	float3& operator *= (const float value) {
		x *= value;
		y *= value;
		z *= value;
		return *this;
	}

	float3& operator /= (const float value) {
		const float inverse = 1.0f / value;
		x *= inverse;
		y *= inverse;
		z *= inverse;
		return *this;
	}

	void normalize(){
		const float lengthInverse = 1.0f / sqrt(x*x + y*y + z*z);
		x *= lengthInverse;
		y *= lengthInverse;
		z *= lengthInverse;
	}

	float length() const {
		return sqrt(x*x + y*y + z*z);
	}

	float lengthSquare( ) const {
		return x*x + y*y + z*z;
	}
};

float3 operator * ( const float value, const float3& vec );



