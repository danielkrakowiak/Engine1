#include "Direct3DUtil.h"

#include <d3d11.h>

using namespace Engine1;

void Direct3DUtil::setResourceName( ID3D11DeviceChild& child, const std::string& name ) {
	child.SetPrivateData( WKPDID_D3DDebugObjectName, name.size(), name.c_str() );
}

int Direct3DUtil::getRefCount( IUnknown& object )
{
	object.AddRef( );
	return object.Release( );
}
