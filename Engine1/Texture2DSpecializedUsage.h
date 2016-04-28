#pragma once

#include <string>
#include <vector>
#include <memory>

#include <d3d11.h>
#include <wrl.h>

#include "Texture2DGeneric.h"

namespace Engine1
{
    template< TexUsage usage, typename PixelType >
    class Texture2DSpecializedUsage
    {};

    template< typename PixelType >
    class Texture2DSpecializedUsage< TexUsage::Immutable, PixelType > 
        : public virtual Texture2DGeneric< PixelType > 
    {};

    template< typename PixelType >
    class Texture2DSpecializedUsage< TexUsage::Dynamic, PixelType > 
        : public virtual Texture2DGeneric< PixelType > 
    {};

    template< typename PixelType >
    class Texture2DSpecializedUsage< TexUsage::Default, PixelType > 
        : public virtual Texture2DGeneric< PixelType > 
    {};
}



