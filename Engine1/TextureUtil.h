#pragma once

#include <memory>
#include <vector>
#include <algorithm>

#include "Texture2DTypes.h"

#include "MathUtil.h"

#include "int3.h"
#include "float2.h"

namespace Engine1
{
    class TextureUtil
    {
        private:

        class TextureBin
        {
            private:

            int2 m_dimensions;
            bool m_horizontal; // Matters only if bin has children.

            std::unique_ptr< TextureBin > m_firstChild, m_secondChild;

            bool m_empty;

            public:

            TextureBin( const int2 textureDimensions, bool empty ) :
                m_dimensions( textureDimensions ),
                m_horizontal( false ),
                m_empty( empty )
            {} // Should check texture dimensions in constructor?

            int2 getDimensions()
            {
                return m_dimensions;
            }

            // Assumption - no texture should be bigger than the bin it is inserted into.
            // Returns true if insertion has succeeded and a position (in pixels) of the inserted texture within the merged texture.
            std::pair< bool, int2 > insert( const int2 textureDimensions )
            {
                if ( !m_empty || m_dimensions.x < textureDimensions.x || m_dimensions.y < textureDimensions.y )
                    return std::make_pair( false, int2::ZERO ); // This bin is already filled or it's too small - cannot insert.

                // This bin has sufficient size.
                // If this bin already has children (is divided) - try to insert in one of them.
                if ( m_firstChild || m_secondChild ) {
                    std::pair< bool, int2 > inserted;
                    if ( m_firstChild )
                        inserted = m_firstChild->insert( textureDimensions );

                    if ( m_secondChild && !inserted.first )
                    {
                        inserted = m_secondChild->insert( textureDimensions );

                        // Add half of bin's width/height to the returned texture's position 
                        // - because it was placed in the second child.
                        if ( inserted.first )
                            inserted.second += m_horizontal ? int2( m_dimensions.x / 2, 0 ) : int2( 0, m_dimensions.y / 2 );
                    }

                    return inserted;
                } else // This bin doesn't have children (isn't divided).
                {
                    if ( m_dimensions.x == textureDimensions.x && m_dimensions.y == textureDimensions.y ) {
                        m_empty = false; // This bin has exactly the size of the texture - fill it.

                        return std::make_pair( true, int2::ZERO );
                    } else {
                        // This bin is too wide - divide horizontally, too high - divide vertically.
                        const bool divideHorizontally = m_dimensions.x > textureDimensions.x;
                        divide( divideHorizontally );

                        // Try to insert again.
                        return insert( textureDimensions );
                    }
                }
            }

            // Can only be called on the root bin (without parent).
            // The bin creates a new enlarged root bin and attaches to it as one of its children.
            // Returns the new root bin.
            static std::unique_ptr< TextureBin > enlarge( std::unique_ptr< TextureBin >& rootBin )
            {
                std::unique_ptr< TextureBin > neighborBin = std::make_unique< TextureBin >( rootBin->m_dimensions, true );

                const bool horizontal = rootBin->m_dimensions.x <= rootBin->m_dimensions.y;
                const int  newWidth   = horizontal ? rootBin->m_dimensions.x * 2 : rootBin->m_dimensions.x;
                const int  newHeight  = !horizontal ? rootBin->m_dimensions.y * 2 : rootBin->m_dimensions.y;

                std::unique_ptr< TextureBin > newRootBin = std::make_unique< TextureBin >( int2( newWidth, newHeight ), true );

                // Attach the children to the new root bin.
                newRootBin->m_horizontal = horizontal;
                newRootBin->m_firstChild = std::move( rootBin ); // Enlarged bin becomes the first child.
                newRootBin->m_secondChild = std::move( neighborBin );

                return newRootBin;
            }

            private:

            void divide( const bool horizontally )
            {
                assert( m_empty ); // Only an empty bin can be divided.

                int2 newDimensions;
                newDimensions.x =  horizontally ? m_dimensions.x / 2 : m_dimensions.x;
                newDimensions.y = !horizontally ? m_dimensions.y / 2 : m_dimensions.y;

                m_horizontal  = horizontally;
                m_firstChild  = std::make_unique< TextureBin >( newDimensions, true );
                m_secondChild = std::make_unique< TextureBin >( newDimensions, true );
            }
        };

        public:

        template< typename PixelType >
        static void copyTextureCpu( 
            Texture2D< PixelType >& destTexture, 
            Texture2D< PixelType >& srcTexture, 
            float2 destTopLeftTexcoords,
            float2 destBottomRightTexcoords,
            float2 srcTopLeftTexcoords, 
            float2 srcBottomRightTexcoords,
            float4 colorMultiplier = float4::ONE )
        {
            if ( srcTopLeftTexcoords.x < 0.0f || srcBottomRightTexcoords.x < srcTopLeftTexcoords.x || srcBottomRightTexcoords.x > 1.0f
                 || srcTopLeftTexcoords.y < 0.0f || srcBottomRightTexcoords.y < srcTopLeftTexcoords.y || srcBottomRightTexcoords.y > 1.0f
                 || destTopLeftTexcoords.x < 0.0f || destBottomRightTexcoords.x < destTopLeftTexcoords.x || destBottomRightTexcoords.x > 1.0f
                 || destTopLeftTexcoords.y < 0.0f || destBottomRightTexcoords.y < destTopLeftTexcoords.y || destBottomRightTexcoords.y > 1.0f ) 
            {
                throw std::exception( "TextureUtil::copyTexture - incorrect region defined." );
            }

            const int2 srcDimensions  = srcTexture.getDimensions();
            const int2 destDimensions = destTexture.getDimensions();

            const int2 srcTopLeft           = (int2)(srcTopLeftTexcoords * (float2)srcDimensions);
            const int2 srcRegionDimensions  = (int2)((srcBottomRightTexcoords - srcTopLeftTexcoords) * (float2)srcDimensions);
            const int2 destTopLeft          = (int2)(destTopLeftTexcoords * (float2)destDimensions);
            const int2 destRegionDimensions = (int2)((destBottomRightTexcoords - destTopLeftTexcoords) * (float2)destDimensions);

            const std::vector< PixelType >& srcData  = srcTexture.getData();
            std::vector< PixelType >&       destData = destTexture.getData();

            if ( srcRegionDimensions == destRegionDimensions && colorMultiplier == float4::ONE )
            {
                // Fast copy, line-by-line is possible, 
                // as source and destination regions have the same dimensions
                // and color multiplier equals (1,1,1,1).
                const int2 dimensions = srcRegionDimensions;

                for ( int y = 0; y < dimensions.y; ++y )
                {
                    // Copy one line of data.
                    std::memcpy( 
                        &destData[ (destTopLeft.y + y) * destDimensions.x + destTopLeft.x ],
                        &srcData[ (srcTopLeft.y + y) * srcDimensions.x + srcTopLeft.x ],
                        dimensions.x * sizeof( PixelType )
                    );
                }
            }
            else
            {
                // Slow re-sampling/re-scaling is required,
                // as source and destination regions have different dimensions.
                const float2 srcTexcoordsSpan = srcBottomRightTexcoords - srcTopLeftTexcoords;

                int2 destPos;
                for ( destPos.y = 0; destPos.y < destRegionDimensions.y; ++destPos.y )
                {
                    for ( destPos.x = 0; destPos.x < destRegionDimensions.x; ++destPos.x )
                    {
                        const float2 srcTexcoords = srcTopLeftTexcoords + srcTexcoordsSpan * ((float2)destPos / (float2)destRegionDimensions);

                        const PixelType srcPixel = srcTexture.sampleBilinearData( srcTexcoords, 0 );

                        destData[ (destTopLeft.y + destPos.y) * destDimensions.x + destTopLeft.x + destPos.x ] = (PixelType)(float4( srcPixel ) * colorMultiplier);
                    }
                }
            }
        }

        // Textures are not rotated during merge (for simplicity).
        // Returns merged texture dimensions and a vector of texcoords 
        // (for top-left and bottom-right corner) describing where input textures 
        // need to be placed within the merged texture.
        // Order of vector of texcoords is the same as the order of input textures. 
        static std::tuple< int2, std::vector< std::pair< float2, float2 > > >
        prepareTextureMerge( const std::vector< int2 >& texturesDimensions )
        {
            for ( auto& dimensions : texturesDimensions ) {
                if ( !MathUtil::isPowerOfTwo( dimensions.x ) || !MathUtil::isPowerOfTwo( dimensions.y ) )
                    throw std::exception( "TextureUtil::prepareTextureMerge - one of the textures does not have power-of-two dimensions." );
            }

            const int textureCount = (int)texturesDimensions.size();

            // Copy input vector, store element's index within input vector as an extra component.
            std::vector< int3 > sortedTextures;
            sortedTextures.reserve( textureCount );
            for( int idx = 0; idx < textureCount; ++idx )
            {
                sortedTextures.emplace_back( 
                    texturesDimensions[ idx ].x, 
                    texturesDimensions[ idx ].y, 
                    idx 
                );
            }

            // Sort textures by increasing dimensions.
            std::sort( 
                sortedTextures.begin(), 
                sortedTextures.end(), 
                []( const int3& texture1, const int3& texture2 )
                { 
                    const int tex1MaxDimension = std::max( texture1.x, texture1.y );
                    const int tex1MinDimension = std::min( texture1.x, texture1.y );
                    const int tex2MaxDimension = std::max( texture2.x, texture2.y );
                    const int tex2MinDimension = std::min( texture2.x, texture2.y );

                    // Note: Handle cases where the maximal dimensions are the same, 
                    // but smaller one are different: Ex. (256, 128) vs (256, 64).
                    if ( tex1MaxDimension > tex2MaxDimension )       return false;
                    else if ( tex2MaxDimension > tex1MaxDimension )  return true;
                    else if ( tex1MinDimension >= tex2MinDimension ) return false;
                    else                                             return true;
                }
            );

            auto rootBin = std::make_unique< TextureBin >( (int2)sortedTextures.back(), false );

            std::vector< int2 > positionsTemp;
            positionsTemp.resize( textureCount, int2::ZERO );

            { // Insert textures in the bin.
                int idx = (int)textureCount - 2;
                while ( idx >= 0 )
                {
                    std::pair< bool, int2 > inserted = rootBin->insert( (int2)sortedTextures[ idx ] );
                    if ( inserted.first )
                    {
                        positionsTemp[ idx ] = inserted.second;

                        --idx; // Insertion succeeded - continue.
                    }
                    else
                    {
                        // Insertion failed - enlarge the root bin.
                        rootBin = TextureBin::enlarge( rootBin ); 
                    }
                }
            }

            // Create a vector of texcoords describing 
            // where to place input texture within the merged texture.
            // (for left-top and bottom-right corners) 
            // (ordered the same as input vector of texture dimensions).
            std::vector< std::pair< float2, float2 > > texcoords;
            texcoords.resize( textureCount );
            for ( int sortedIdx = 0; sortedIdx < textureCount; ++sortedIdx )
            {
                const int idxInInputVector = sortedTextures[ sortedIdx ].z;

                // Top-left corner texcoords.
                texcoords[ idxInInputVector ].first  
                    = (float2)positionsTemp[ sortedIdx ] / (float2)rootBin->getDimensions();

                // Bottom-right corner texcoords.
                texcoords[ idxInInputVector ].second 
                    = (float2)( positionsTemp[ sortedIdx ] + texturesDimensions[ idxInInputVector ] ) / (float2)rootBin->getDimensions();
            }

            return std::make_tuple( rootBin->getDimensions(), texcoords );
        }

        // Textures are not rotated during merge (for simplicity).
        // Returns the merged texture or nullptr if merge failed.
        template< typename PixelType >
        static std::shared_ptr< RenderTargetTexture2D< PixelType > >
        mergeTextures( 
            const std::vector< std::shared_ptr< Texture2D< PixelType > > >& textures,
            const std::vector< float4 >& colorMultipliers,
            const std::vector< std::pair< float2, float2 > >& texcoords, 
            const int2 mergedTextureDimensions,
            ID3D11Device3& device,
            DXGI_FORMAT textureFormat, 
            DXGI_FORMAT viewFormat )
        {
            if ( textures.empty() || texcoords.empty() || colorMultipliers.empty() )
                throw std::exception( "TextureUtil::mergeTextures - no input textures, texcoords or color multipleirs were passed." );

            if ( textures.size() != texcoords.size() || textures.size() != colorMultipliers.size() )
                throw std::exception( "TextureUtil::mergeTextures - input texture count is different than number of texcoords or color multipliers." );

            for ( auto& texture : textures ) {
                if ( !MathUtil::isPowerOfTwo( texture->getWidth() ) || !MathUtil::isPowerOfTwo( texture->getHeight() ) )
                    throw std::exception( "TextureUtil::mergeTextures - one of the textures does not have power-of-two dimensions." );
            }

            const int textureCount = (int)textures.size();

            // Create a new texture.
            auto mergedTexture = std::make_shared< RenderTargetTexture2D< PixelType > >
                ( device, mergedTextureDimensions.x, mergedTextureDimensions.y, true, false, false, textureFormat, viewFormat, viewFormat, viewFormat );

            // Copy input textures to the merged texture (with up-scaling).
            for ( int idx = 0; idx < textureCount; ++idx ) 
            {
                TextureUtil::copyTextureCpu(
                    *mergedTexture,
                    *textures[ idx ],
                    texcoords[ idx ].first,
                    texcoords[ idx ].second,
                    float2::ZERO,
                    float2::ONE,
                    colorMultipliers[ idx ]
                );
            }

            return mergedTexture;
        }

        template< typename PixelType >
        static std::string getDescription( const Texture2D< PixelType >& texture, const bool printPath = true, const bool printDimensions = true )
        {
            std::string text;

            if ( !printPath && !printDimensions )
                return text;

            if ( printPath )
                text += texture.getFileInfo().getPath() + " ";

            if ( printDimensions )
                text += std::to_string( texture.getWidth() ) + "/" + std::to_string( texture.getHeight() );

            text += "\n";

            return text;
        }
    };
};

