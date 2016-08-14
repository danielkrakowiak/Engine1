#include "ComplexModel.h"

using namespace Engine1;

ComplexModel::ComplexModel() {}


ComplexModel::~ComplexModel() {}

std::shared_ptr<BlockModel>& ComplexModel::getModelLOD( float distance ){
	std::vector<BlockModelLOD>::reverse_iterator beginIt = m_modelLODs.rend();

	for ( std::vector<BlockModelLOD>::reverse_iterator it = m_modelLODs.rbegin( ); it != beginIt; ++it ) {
		if ( ( *it ).getDistance() < distance ) return ( *it ).getModel();
	}

	return m_modelLODs.front().getModel();
}

std::shared_ptr<BlockMesh>& ComplexModel::getShadowMeshLOD( float distance ){
	std::vector<BlockMeshLOD>::reverse_iterator beginIt = m_shadowMeshLODs.rend( );

	for ( std::vector<BlockMeshLOD>::reverse_iterator it = m_shadowMeshLODs.rbegin( ); it != beginIt; ++it ) {
		if ( ( *it ).getDistance( ) < distance ) return ( *it ).getMesh( );
	}

	return m_shadowMeshLODs.front( ).getMesh( );
}