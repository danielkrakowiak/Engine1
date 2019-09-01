#include "DX11Util.h"

#include <d3d11_3.h>

using namespace Engine1;

void DX11Util::setResourceName( ID3D11DeviceChild& child, const std::string& name ) 
{
	child.SetPrivateData( WKPDID_D3DDebugObjectName, (unsigned int)name.size(), name.c_str() );
}

int DX11Util::getRefCount( IUnknown& object )
{
	object.AddRef( );
	return object.Release( );
}

std::string DX11Util::getLastErrorMessage()
{
    /*ULONG bufSize = 512;
    CHAR pBuf[512];

    LPTSTR pTemp = NULL;

    DWORD retSize = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
    NULL, GetLastError(), LANG_ENGLISH, (LPTSTR)&pTemp, 0, NULL );*/

    //if ( !retSize || !pTemp ) {
    //    pBuf[ 0 ] = '\0';
    //} else {
    //    pTemp[ strlen( pTemp ) - 2 ] = '\0'; //remove cr and newline character
    //    sprintf( pBuf, "%0.*s (0x%x)", bufSize - 16, pTemp, GetLastError() );
    //    LocalFree( (HLOCAL)pTemp );
    //}

    return "NOT YET IMPLEMENTED";
}
