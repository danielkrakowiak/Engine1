#include "BlockModelLOD.h"

using namespace Engine1;

BlockModelLOD::BlockModelLOD( float distance, std::shared_ptr<BlockModel>& model ) {
	this->m_distance = distance;
	this->m_model = model;
}


BlockModelLOD::~BlockModelLOD() {}
