#pragma once

#include <string>
#include <vector>
#include <memory>

#include <d3d11_3.h>
#include <wrl.h>

#include "Texture2DGeneric.h"

namespace Engine1
{
    template< TexUsage usage, typename PixelType >
    class Texture2DSpecUsage
    {};

    template< typename PixelType >
    class Texture2DSpecUsage< TexUsage::Immutable, PixelType > 
        : public virtual Texture2DGeneric< PixelType > 
    {};

    template< typename PixelType >
    class Texture2DSpecUsage< TexUsage::Dynamic, PixelType > 
        : public virtual Texture2DGeneric< PixelType > 
    {};

    template< typename PixelType >
    class Texture2DSpecUsage< TexUsage::Default, PixelType > 
        : public virtual Texture2DGeneric< PixelType > 
    {};
}



