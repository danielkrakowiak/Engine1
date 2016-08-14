#include "BlockMeshLOD.h"

using namespace Engine1;

BlockMeshLOD::BlockMeshLOD( float distance, std::shared_ptr<BlockMesh>& mesh ) {
	this->m_distance = distance;
	this->m_mesh = mesh;
}


BlockMeshLOD::~BlockMeshLOD() {}
