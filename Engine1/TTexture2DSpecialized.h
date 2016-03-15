#pragma once

#include <string>
#include <vector>
#include <memory>

#include <d3d11.h>
#include <wrl.h>

#include "TTexture2DGeneric.h"

namespace Engine1
{
    template< TexUsage usage, typename PixelType >
    class TTexture2DSpecialized
        : public TTexture2DGeneric< PixelType >
    {
        public:

        template< TexUsage _u = usage >
        void loadCpuToGpu( ID3D11Device& device, ID3D11DeviceContext& deviceContext, 
                           typename std::enable_if<
                               _u == TexUsage::Immutable ||
                               _u == TexUsage::Dynamic ||
                               _u == TexUsage::Default ||
                               _u == TexUsage::StagingReadWrite ||
                               _u == TexUsage::StagingWrite
                           >::type* = 0 )
        {
            TTexture2DGeneric::loadCpuToGpu( device, deviceContext );
        }

        template< TexUsage _u = usage >
        void loadGpuToCpu( ID3D11DeviceContext& deviceContext, 
                           typename std::enable_if<
                               _u == TexUsage::StagingRead ||
                               _u == TexUsage::StagingReadWrite
                           >::type* = 0 )
        {
            TTexture2DGeneric::loadGpuToCpu( deviceContext );
        }

        // #TODO: only some configurations support this method. Add std::enable_if.
        void unloadFromCpu()
        {
            TTexture2DGeneric::unloadFromCpu();
        }

        // #TODO: only some configurations support this method. Add std::enable_if.
        void unloadFromGpu()
        {
            TTexture2DGeneric::unloadFromGpu();
        }
    };
}



