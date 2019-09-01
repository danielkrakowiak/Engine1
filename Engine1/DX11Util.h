#include <string>

struct ID3D11DeviceChild;
struct IUnknown;

namespace Engine1
{
    namespace DX11Util
    {

        void setResourceName( ID3D11DeviceChild& child, const std::string& name );

        int getRefCount( IUnknown& object );

        std::string getLastErrorMessage();
    };
}