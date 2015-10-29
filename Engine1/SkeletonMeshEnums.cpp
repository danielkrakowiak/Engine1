#include "SkeletonMeshEnums.h"

const std::array<BonesPerVertexCount::Type, 4> BonesPerVertexCount::values = {
	BonesPerVertexCount::Type::ZERO, 
	BonesPerVertexCount::Type::ONE,
	BonesPerVertexCount::Type::TWO,
	BonesPerVertexCount::Type::FOUR
};

const std::array<BonesPerVertexCount::Type, 3> BonesPerVertexCount::correctValues = {
	BonesPerVertexCount::Type::ONE,
	BonesPerVertexCount::Type::TWO,
	BonesPerVertexCount::Type::FOUR
};