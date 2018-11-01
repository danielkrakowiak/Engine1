#pragma once

#include <string>

#include "Texture2DFileInfo.h"
#include "int2.h"

namespace Engine1
{
    template< typename PixelType >
    class TextureBase : public Asset
    {
        public:

        virtual ~TextureBase() = default;

        virtual void saveToFile( 
            const std::string path, 
            const Texture2DFileInfo::Format format, 
            bool flipRedBlue = false, 
            bool removeAlpha = false, 
            bool flipVertically = false ) const;

        virtual int getWidth() const = 0;
        virtual int getHeight() const = 0;

        virtual const std::vector< PixelType >& getData() const = 0;
        virtual std::vector< PixelType >& getData() = 0;

        virtual bool isInCpuMemory() const = 0;
        virtual bool isInGpuMemory() const = 0;

        protected:

        TextureBase() = default;

        int formatToFreeImagePlusSaveFlag( Texture2DFileInfo::Format format ) const;
    };

    template< typename PixelType >
    void TextureBase< PixelType >
        ::saveToFile( 
            const std::string path, 
            const Texture2DFileInfo::Format format, 
            bool flipRedBlue, 
            bool removeAlpha, 
            bool flipVertically ) const
    {
        if ( path.empty() )
            throw std::exception( "Texture2DGeneric::saveToFile - path cannot be empty." );

        if ( !isInCpuMemory() )
            throw std::exception( "Texture2DGeneric::saveToFile - texture not in CPU memory." );

        // #TODO: Could be improved significantly by avoiding copying memory to FreeImage image. Is it possible?

        // Can only flip Red, Blue channels if they exist.
        flipRedBlue = (sizeof( PixelType ) >= 3 ? flipRedBlue : false);
        removeAlpha = (sizeof( PixelType ) == 4 ? removeAlpha : false);

        auto fipImagePixelSize = static_cast<int>( sizeof( PixelType ) );
        if (fipImagePixelSize == 4 && removeAlpha) {
            --fipImagePixelSize;
        }

        if ( fipImagePixelSize > 4 )
            throw std::exception( "TextureBase::saveToFile - unsupported pixel size (more than 4 bytes per pixel)." );

        const auto width = getWidth();
        const auto height = getHeight();

        fipImage image( FIT_BITMAP, getWidth(), getHeight(), fipImagePixelSize * 8 );

        const bool isPadded = image.getWidth() * fipImagePixelSize != image.getScanWidth();

        if ( !isPadded && !flipRedBlue && !removeAlpha && !flipVertically ) 
        {
            // Copy entire image.
            std::memcpy( image.accessPixels(), getData().data(), getWidth() * getHeight() * sizeof( PixelType ) );
        } 
        else if ( isPadded && !flipRedBlue && !removeAlpha)
        {
            // Copy image - line by line to account for padding at the end of each line.
            if (!flipVertically)
            {
                for ( int y = 0; y < height; ++y )
                    std::memcpy( image.getScanLine( y ), getData().data() + y * width, width * sizeof( PixelType ) );
            }
            else
            {
                for ( int y = 0; y < height; ++y )
                    std::memcpy( image.getScanLine( height - y - 1 ), getData().data() + y * width, width * sizeof( PixelType ) );
            }
        }
        else if (flipRedBlue)
        {
            if (fipImagePixelSize == 3)
            {
                for ( auto y = 0; y < height; ++y ) 
                {
                    const auto destY = (flipVertically ? height - y - 1 : y);

                    for ( auto x = 0; x < width; ++x ) 
                    {
                        const auto* srcPixelPtr     = getData().data() + y * width + x;
                        const auto* srcPixelBytePtr = reinterpret_cast< const BYTE* >( srcPixelPtr );
                        auto* destPixelBytePtr      = image.getScanLine( destY ) + x * fipImagePixelSize;

                        for ( auto i = 0; i < fipImagePixelSize; ++i )
                            destPixelBytePtr[ i ] = srcPixelBytePtr[ 2 - i ]; // Copy pixel and flip red with blue.
                    }
                }
            }
            else if (fipImagePixelSize == 4)
            {
                for ( auto y = 0; y < height; ++y ) 
                {
                    const auto destY = (flipVertically ? height - y - 1 : y);

                    for ( auto x = 0; x < width; ++x ) 
                    {
                        const auto* srcPixelPtr     = getData().data() + y * width + x;
                        const auto* srcPixelBytePtr = reinterpret_cast< const BYTE* >( srcPixelPtr );
                        auto* destPixelBytePtr      = image.getScanLine( destY ) + x * fipImagePixelSize;

                        for ( auto i = 0; i < fipImagePixelSize; ++i )
                            destPixelBytePtr[ i ] = srcPixelBytePtr[ 2 - i ]; // Copy pixel RGB and flip red with blue.

                        destPixelBytePtr[ 3 ] = srcPixelBytePtr[ 3 ]; // Copy alpha.
                    }
                }
            }
        }
        else if (!flipRedBlue)
        {
            for ( auto y = 0; y < height; ++y ) 
            {
                const auto destY = (flipVertically ? height - y - 1 : y);

                for ( auto x = 0; x < width; ++x ) 
                {
                    const auto* srcPixelPtr     = getData().data() + y * width + x;
                    const auto* srcPixelBytePtr = reinterpret_cast< const BYTE* >( srcPixelPtr );
                    auto* destPixelBytePtr      = image.getScanLine( destY ) + x * fipImagePixelSize;

                    for ( auto i = 0; i < fipImagePixelSize; ++i )
                    {
                        destPixelBytePtr[ i ] = srcPixelBytePtr[ i ]; // Copy pixel.
                    }
                }
            }
        }
        else
        {
            assert(false);
        }

        if ( !image.save( path.c_str(), formatToFreeImagePlusSaveFlag( format ) ) )
            throw std::exception( "Texture2DGeneric::saveToFile - saving texture failed." );
    }

    template< typename PixelType >
    int TextureBase< PixelType >
        ::formatToFreeImagePlusSaveFlag( const Texture2DFileInfo::Format format ) const
    {
        //#TODO: How about quality of the output file? Compression etc?

        if ( format == Texture2DFileInfo::Format::BMP )
            return BMP_DEFAULT;
        else if ( format == Texture2DFileInfo::Format::JPEG )
            return JPEG_QUALITYNORMAL;
        else if ( format == Texture2DFileInfo::Format::PNG )
            return PNG_Z_DEFAULT_COMPRESSION;
        else if ( format == Texture2DFileInfo::Format::TIFF )
            return TIFF_DEFAULT;
        else {
            assert( false );
            return 0;
        }
    }
}

