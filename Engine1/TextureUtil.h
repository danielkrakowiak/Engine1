#pragma once

#include <memory>
#include <vector>
#include <algorithm>

#include "Texture2D.h"

#include "MathUtil.h"

#include "int2.h"
#include "float2.h"

namespace Engine1
{
    class TextureUtil
    {
        private:

        template< typename PixelType >
        class TextureBin
        {
            private:

            int m_width;
            int m_height;
            bool m_horizontal; // Matters only if bin has children.

            //std::weak_ptr< TextureBin > m_parent;
            std::unique_ptr< TextureBin > m_firstChild, m_secondChild;

            // Note: Texture has to fill the entire space of the bin.
            std::shared_ptr< Texture2DGeneric< PixelType > > m_texture;

            public:

            TextureBin( const int width, const int height, const std::shared_ptr< Texture2DGeneric< PixelType > > texture ) :
                //m_parent( parent ),
                m_width( width ),
                m_height( height ),
                m_horizontal( false ),
                m_texture( texture )
            {} // Should check texture dimensions in constructor?

            TextureBin( const int width, const int height ) :
                m_width( width ),
                m_height( height ),
                m_horizontal( false )
            {}

            int getWidth()
            {
                return m_width;
            }

            int getHeight()
            {
                return m_height;
            }

            // Assumption - no texture should be bigger than the bin it is inserted into.
            // Returns true if insertion has succeeded and a position (in pixels) of the inserted texture within the merged texture.
            std::pair< bool, int2 > insert( std::shared_ptr< Texture2DGeneric< PixelType > > texture )
            {
                if ( m_texture || m_width < texture->getWidth() || m_height < texture->getHeight() )
                    return std::make_pair( false, int2::ZERO ); // This bin is already filled or it's too small - cannot insert.

                // This bin has sufficient size.
                // If this bin already has children (is divided) - try to insert in one of them.
                if ( m_firstChild || m_secondChild ) {
                    std::pair< bool, int2 > inserted;
                    if ( m_firstChild )
                        inserted = m_firstChild->insert( texture );

                    if ( m_secondChild && !inserted.first )
                    {
                        inserted = m_secondChild->insert( texture );

                        // Add half of bin's width/height to the returned texture's position 
                        // - because it was placed in the second child.
                        if ( inserted.first )
                            inserted.second += m_horizontal ? int2( m_width / 2, 0 ) : int2( 0, m_height / 2 );
                    }

                    return inserted;
                } else // This bin doesn't have children (isn't divided).
                {
                    if ( m_width == texture->getWidth() && m_height == texture->getHeight() ) {
                        m_texture = texture; // This bin has exactly the size of the texture - fill it.

                        return std::make_pair( true, int2::ZERO );
                    } else {
                        // This bin is too wide - divide horizontally, too high - divide vertically.
                        const bool divideHorizontally = m_width > texture->getWidth();
                        divide( divideHorizontally );

                        // Try to insert again.
                        return insert( texture );
                    }
                }
            }

            // Can only be called on the root bin (without parent).
            // The bin creates a new enlarged root bin and attaches to it as one of its children.
            // Returns the new root bin.
            static std::unique_ptr< TextureBin > enlarge( std::unique_ptr< TextureBin >& rootBin )
            {
                std::unique_ptr< TextureBin > neighborBin = std::make_unique< TextureBin >( rootBin->m_width, rootBin->m_height );

                const bool horizontal = rootBin->m_width <= rootBin->m_height;
                const int  newWidth = horizontal ? rootBin->m_width * 2 : rootBin->m_width;
                const int  newHeight = !horizontal ? rootBin->m_height * 2 : rootBin->m_height;

                std::unique_ptr< TextureBin > newRootBin = std::make_unique< TextureBin >( newWidth, newHeight );

                // Attach the children to the new root bin.
                newRootBin->m_horizontal = horizontal;
                newRootBin->m_firstChild = std::move( rootBin ); // Enlarged bin becomes the first child.
                newRootBin->m_secondChild = std::move( neighborBin );

                return newRootBin;
            }

            // Returns pair( false, (0, 0) ) if there is no such texture in the bin and its children.
            // Otherwise returns position of the given texture in the newly merged texture (in pixel from the left-top corner).
            std::pair< bool, int2 > getTexturePosition( const std::shared_ptr< Texture2DGeneric< PixelType > > texture )
            {
                // If this bin holds that texture.
                if ( m_texture == texture )
                    return std::make_pair( true, int2( 0, 0 ) );

                if ( m_firstChild ) {
                    auto result = m_firstChild->getTexturePosition( texture );
                    if ( result.first )
                        return result;
                }

                if ( m_secondChild ) {
                    auto result = m_secondChild->getTexturePosition( texture );
                    if ( result.first ) {
                        result.second += m_horizontal ? int2( m_width / 2, 0 ) : int2( 0, m_height / 2 );
                        return result;
                    }
                }

                // Neither this bin or its children hold the given texture.
                return std::make_pair( false, int2( 0, 0 ) );
            }

            private:

            void divide( const bool horizontally )
            {
                assert( !m_texture ); // Only an empty bin can be divided.

                const int newWidth = horizontally ? m_width / 2 : m_width;
                const int newHeight = !horizontally ? m_height / 2 : m_height;

                m_horizontal = horizontally;
                m_firstChild = std::make_unique< TextureBin >( newWidth, newHeight, nullptr );
                m_secondChild = std::make_unique< TextureBin >( newWidth, newHeight, nullptr );
            }
        };

        public:

        // Describes where a texture is placed within a merged texture.
        class TexturePlacement
        {
            public:

            static bool areTexcoordsEqual( const TexturePlacement& placement1, const TexturePlacement& placement2 )
            {
                return MathUtil::areEqual( placement1.getTopLeftTexcoords(), placement2.getTopLeftTexcoords(), 0.0f, 0.0001f )
                    && MathUtil::areEqual( placement1.getBottomRightTexcoords(), placement2.getBottomRightTexcoords(), 0.0f, 0.0001f );
            }

            TexturePlacement( const float2& topLeftTexcoords, const float2& bottomRightTexcoords ) :
                m_topLeftTexcoords( topLeftTexcoords ),
                m_bottomRightTexcoords( bottomRightTexcoords )
            {}

            float2 getTopLeftTexcoords() const
            {
                return m_topLeftTexcoords;
            }

            float2 getBottomRightTexcoords() const
            {
                return m_bottomRightTexcoords;
            }

            float2 getDimensionsInTexcoords() const
            {
                return m_bottomRightTexcoords - m_topLeftTexcoords;
            }

            private:

            float2 m_topLeftTexcoords;
            float2 m_bottomRightTexcoords;
        };

        template< typename PixelType >
        static void copyTexture( Texture2DGeneric< PixelType >& destTexture, Texture2DGeneric< PixelType >& srcTexture, int2 destTopLeft, int2 srcTopLeft, int2 dimensions )
        {
            if ( srcTopLeft.x < 0 || dimensions.x < 0 || srcTopLeft.x + dimensions.x > srcTexture.getWidth()
                 || srcTopLeft.y < 0 || dimensions.y < 0 || srcTopLeft.y + dimensions.y > srcTexture.getHeight()
                 || destTopLeft.x < 0 || destTopLeft.x + dimensions.x > destTexture.getWidth()
                 || destTopLeft.y < 0 || destTopLeft.y + dimensions.y > destTexture.getHeight() ) 
            {
                throw std::exception( "TextureUtil::copyTexture - incorrect region defined." );
            }

            const std::vector< PixelType >& srcData  = srcTexture.getData();
            std::vector< PixelType >&       destData = destTexture.getData();

            const int srcWidth  = srcTexture.getWidth();
            const int destWidth = destTexture.getWidth();

            for ( int y = 0; y < dimensions.y; ++y )
            {
                // Copy one line of data.
                std::memcpy( 
                    &destData[ (destTopLeft.y + y) * destWidth + destTopLeft.x ],
                    &srcData[ (srcTopLeft.y + y) * srcWidth + srcTopLeft.x ],
                    dimensions.x * sizeof( PixelType )
                );
            }
        }

        // Every texture has to have power-of-two dimensions.
        // Textures are not rotated during merge (for simplicity).
        // Returns the merged texture and a vector of positions 
        // (left-top corner position in pixels) at which input textures
        // were placed within the merged texture. 
        // Order of vector of positions is the same as order of input textures. 
        template< typename PixelType >
        static std::tuple< 
            std::shared_ptr< Texture2D< TexUsage::Default, TexBind::ShaderResource, PixelType > >,
            std::vector< TexturePlacement >
        > mergeTextures( const std::vector< std::shared_ptr< Texture2DGeneric< PixelType > > >& inputTextures,
                         ID3D11Device3& device, DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat )
        {
            if ( inputTextures.empty() )
                throw std::exception( "TextureUtil::mergeTextures - no input textures were passed." );

            for ( auto& texture : inputTextures ) {
                if ( !MathUtil::isPowerOfTwo( texture->getWidth() ) || !MathUtil::isPowerOfTwo( texture->getHeight() ) )
                    throw std::exception( "TextureUtil::mergeTextures - one of the texture does not have power-of-two dimensions." );
            }

            // Check if all input textures are the same.
            bool allInputTexturesAreTheSame = true;
            for ( const auto& inputTexture : inputTextures ) {
                if ( inputTexture != inputTextures.front() )
                    allInputTexturesAreTheSame = false;
            }


            std::vector< std::shared_ptr< Texture2DGeneric< PixelType > > > textures( inputTextures.begin(), inputTextures.end() );

            int maxWidth = 1, maxHeight = 1;

            // Find maximal dimensions of the texture set.
            for ( auto& texture : textures ) {
                maxWidth  = std::max( maxWidth, texture->getWidth() );
                maxHeight = std::max( maxHeight, texture->getHeight() );
            }

            // Sort textures by increasing dimensions.
            if ( !allInputTexturesAreTheSame )
            {
                std::sort( 
                    textures.begin(), 
                    textures.end(), 
                    []( const std::shared_ptr< Texture2DGeneric< PixelType > >& tex1, const std::shared_ptr< Texture2DGeneric< PixelType > >& tex2 )
                    { 
                        const int tex1MaxDimension = std::max( tex1->getWidth(), tex1->getHeight() );
                        const int tex1MinDimension = std::min( tex1->getWidth(), tex1->getHeight() );
                        const int tex2MaxDimension = std::max( tex2->getWidth(), tex2->getHeight() );
                        const int tex2MinDimension = std::min( tex2->getWidth(), tex2->getHeight() );

                        // Note: Handle cases where the maximal dimensions are the same, 
                        // but smaller one are different: Ex. (256, 128) vs (256, 64).
                        if ( tex1MaxDimension > tex2MaxDimension )       return false;
                        else if ( tex2MaxDimension > tex1MaxDimension )  return true;
                        else if ( tex1MinDimension >= tex2MinDimension ) return false;
                        else                                             return true;
                    }
                );
            }

            std::shared_ptr< Texture2DGeneric< PixelType > > texture = textures.back();

            std::unique_ptr< TextureBin< PixelType > > rootBin 
                = std::make_unique< TextureBin< PixelType > >( texture->getWidth(), texture->getHeight(), texture );

            std::vector< int2 > texturePositions; // In the same order as ordered textures vector.
            texturePositions.resize( inputTextures.size(), int2::ZERO );

            { // Insert textures in the bin.
                int idx = (int)textures.size() - 2;
                while ( idx >= 0 )
                {
                    texture = textures[ idx ];

                    std::pair< bool, int2 > inserted = rootBin->insert( texture );
                    if ( inserted.first )
                    {
                        texturePositions[ idx ] = inserted.second;

                        --idx; // Insertion succeeded - continue.
                    }
                    else
                    {
                        rootBin = TextureBin< PixelType >::enlarge( rootBin ); // Insertion failed - enlarge the root bin.
                    }
                }
            }

            // Create new texture.
            auto mergedTexture = std::make_shared< Texture2D< TexUsage::Default, TexBind::ShaderResource, PixelType > >
                ( device, rootBin->getWidth(), rootBin->getHeight(), true, false, false, textureFormat, viewFormat );

            std::vector< TexturePlacement > texturePlacements;
            texturePlacements.reserve( inputTextures.size() );

            // Copy data from source textures to the merged texture.
            // Collect texture positions withing the merged texture.
            for ( int idx = 0; idx < inputTextures.size(); ++idx )
            {
                const auto& inputTexture = inputTextures[ idx ];

                const int idxWithinSorted = allInputTexturesAreTheSame ? idx : (int)( std::find( textures.begin(), textures.end(), inputTexture ) - textures.begin() );
                int2& texturePosition = texturePositions[ idxWithinSorted ];

                TextureUtil::copyTexture( 
                    *mergedTexture, 
                    *inputTexture, 
                    texturePosition, 
                    int2::ZERO, 
                    int2( inputTexture->getWidth(), inputTexture->getHeight() ) 
                );

                texturePlacements.push_back( 
                    TexturePlacement( 
                        float2( (float)texturePosition.x / mergedTexture->getWidth(), (float)texturePosition.y / mergedTexture->getHeight() ),
                        float2( (float)(texturePosition.x + inputTexture->getWidth()) / mergedTexture->getWidth(), (float)(texturePosition.y + inputTexture->getHeight()) / mergedTexture->getHeight() )
                     ) 
                );
            }

            //mergedTexture.saveToFile( "Assets/Textures/merged.png", Texture2DFileInfo::Format::PNG );

            return std::make_tuple( mergedTexture, texturePlacements );
        }

        // Every texture has to have power-of-two dimensions.
        // Textures are not rotated during merge (for simplicity).
        // Returns the merged texture or nullptr if merge failed.
        template< typename PixelType >
        static std::shared_ptr< Texture2D< TexUsage::Default, TexBind::ShaderResource, PixelType > >
        mergeTextures( const std::vector< std::shared_ptr< Texture2DGeneric< PixelType > > >& inputTextures,
                       const std::vector< TexturePlacement >& placements, const bool duplicateTextures, ID3D11Device3& device,
                       DXGI_FORMAT textureFormat, DXGI_FORMAT viewFormat )
        {
            if ( inputTextures.empty() || placements.empty() )
                throw std::exception( "TextureUtil::mergeTextures - no input textures or no placements were passed." );

            if ( !duplicateTextures && inputTextures.size() != placements.size() )
                throw std::exception( "TextureUtil::mergeTextures - input texture count is different than number of placement positions." );

            if ( duplicateTextures && inputTextures.size() != 1 )
                throw std::exception( "TextureUtil::mergeTextures - there can be only one input texture to be duplicated." );

            for ( auto& texture : inputTextures ) {
                if ( !MathUtil::isPowerOfTwo( texture->getWidth() ) || !MathUtil::isPowerOfTwo( texture->getHeight() ) )
                    throw std::exception( "TextureUtil::mergeTextures - one of the texture does not have power-of-two dimensions." );
            }

            const int textureCount = duplicateTextures ? (int)placements.size() : (int)inputTextures.size();

            // Calculate merged texture dimensions.
            int2 mergeTextureDimensions;
            mergeTextureDimensions.x = (int)( (float)inputTextures[ 0 ]->getWidth()  * ( 1.0f / placements[ 0 ].getDimensionsInTexcoords().x ) );
            mergeTextureDimensions.y = (int)( (float)inputTextures[ 0 ]->getHeight() * ( 1.0f / placements[ 0 ].getDimensionsInTexcoords().y ) );

            // Check if other textures need the same merged texture dimensions.
            for ( int i = 1; i < textureCount; ++i )
            {
                const int texIdx = std::min( (int)inputTextures.size() - 1, i );
                int2 dimensions;
                dimensions.x = (int)((float)inputTextures[ texIdx ]->getWidth()  * (1.0f / placements[ i ].getDimensionsInTexcoords().x));
                dimensions.y = (int)((float)inputTextures[ texIdx ]->getHeight() * (1.0f / placements[ i ].getDimensionsInTexcoords().y));

                if ( dimensions != mergeTextureDimensions )
                    return nullptr; // Merge was not possible - incorrect texture placement.
            }

            // #TODO: Need to check if all textures suggest the same merged texture size.

            // Check if input textures would overlap in the merged texture.
            //for ( int i = 0; i < inputTextures.size(); ++i ) 
            //{
            //    const auto&   texture1      = inputTextures[ i ];
            //    const float2& minTexCoords1 = placements[ i ];
            //    const float2& maxTexCoords1 = minTexCoords1 + float2( (float)texture1->getWidth() / (float)mergeTextureDimensions.x,
            //                                                          (float)texture1->getHeight() / (float)mergeTextureDimensions.y );

            //    for ( int j = 0; j < inputTextures.size(); ++j ) 
            //    {
            //        if ( i == j )
            //            continue; // Don't check overlap with itself.

            //        const auto&   texture2      = inputTextures[ j ];
            //        const float2& minTexCoords2 = placements[ j ];
            //        const float2& maxTexCoords2 = minTexCoords2 + float2( (float)texture2->getWidth() / (float)mergeTextureDimensions.x,
            //                                                              (float)texture2->getHeight() / (float)mergeTextureDimensions.y );

            //        if ( minTexCoords1 < maxTexCoords2 || minTexCoords2 < maxTexCoords1 )
            //            return nullptr; // Textures would overlap - merge is not possible.
            //    }
            //}

            // Create new texture.
            auto mergedTexture = std::make_shared< Texture2D< TexUsage::Default, TexBind::ShaderResource, PixelType > >
                ( device, mergeTextureDimensions.x, mergeTextureDimensions.y, true, false, false, textureFormat, viewFormat );

            for ( int idx = 0; idx < textureCount; ++idx ) 
            {
                const int     texIdx = std::min( (int)inputTextures.size() - 1, idx );
                const auto&   inputTexture = inputTextures[ texIdx ];
                const float2& placement    = placements[ idx ].getTopLeftTexcoords();

                TextureUtil::copyTexture(
                    *mergedTexture,
                    *inputTexture,
                    int2( (int)(placement.x * (float)mergeTextureDimensions.x), (int)(placement.y * (float)mergeTextureDimensions.y) ),
                    int2::ZERO,
                    inputTexture->getDimensions()
                );
            }

            return mergedTexture;
        }

        template< typename PixelType >
        static std::string getDescription( const Texture2DGeneric< PixelType >& texture, const bool printPath = true, const bool printDimensions = true )
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

