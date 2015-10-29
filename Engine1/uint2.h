#pragma once

class uint2 {
	typedef unsigned int uint2_type;

public:
	uint2_type x;
	uint2_type y;

	uint2( ) {}
	uint2( const uint2& a ) : x( a.x ), y( a.y ) {}

	uint2( uint2_type x, uint2_type y ) : x( x ), y( y ) {}

	static int size( ) {
		return 2;
	}

	uint2_type* getData() {
		return &x;
	}

	bool operator == ( const uint2& vec ) const {
		return x == vec.x && y == vec.y;
	}

	bool operator != ( const uint2& vec ) const {
		return x != vec.x || y != vec.y;
	}
};