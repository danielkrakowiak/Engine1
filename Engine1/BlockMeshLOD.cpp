#include "BlockMeshLOD.h"


BlockMeshLOD::BlockMeshLOD( float distance, std::shared_ptr<BlockMesh>& mesh ) {
	this->distance = distance;
	this->mesh = mesh;
}


BlockMeshLOD::~BlockMeshLOD() {}
