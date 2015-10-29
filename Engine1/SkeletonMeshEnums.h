#pragma once

#include <array>

struct BonesPerVertexCount
{
	enum class Type : unsigned char
	{
		ZERO = 0,
		ONE = 1,
		TWO = 2,
		FOUR = 4
	};

	static const std::array<Type, 4> values;
	static const std::array<Type, 3> correctValues;
};