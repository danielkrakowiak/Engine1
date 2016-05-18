#include "ModelTexture2DParser.h"

using namespace Engine1;

template<>
DXGI_FORMAT ModelTexture2DParser< uchar4 >::getDefaultFormat()
{
    return DXGI_FORMAT_B8G8R8A8_UNORM;
}

template<>
DXGI_FORMAT ModelTexture2DParser< unsigned char >::getDefaultFormat()
{
    return DXGI_FORMAT_R8_UNORM;
}
