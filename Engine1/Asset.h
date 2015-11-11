#pragma once

#include <string>

class Asset {

	public:

	enum class Type : char
	{
		BlockMesh         = 0,
		SkeletonMesh      = 1,
		Texture2D         = 2,
		BlockModel        = 3,
		SkeletonModel     = 4,
		SkeletonAnimation = 5
	};

	virtual Type getType() const = 0;
};

