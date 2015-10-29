#include "BlockModelLOD.h"


BlockModelLOD::BlockModelLOD( float distance, std::shared_ptr<BlockModel>& model ) {
	this->distance = distance;
	this->model = model;
}


BlockModelLOD::~BlockModelLOD() {}
