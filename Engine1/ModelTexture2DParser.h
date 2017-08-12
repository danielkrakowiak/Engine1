#pragma once

#include <vector>
#include <memory>
#include <d3d11_3.h>

#include "ModelTexture2D.h"
#include "BinaryFile.h"

namespace Engine1
{
    template< typename PixelType >
    class ModelTexture2DParser
    {
        public:

        static std::shared_ptr< ModelTexture2D< PixelType > > parseBinary( std::vector< char >::const_iterator& dataIt, const bool loadRecurrently, ID3D11Device3& device );
        static void                                           writeBinary( std::vector< char >& data, const ModelTexture2D< PixelType >& modelTexture );

        static DXGI_FORMAT getDefaultFormat();
    };

    template< typename PixelType >
    std::shared_ptr< ModelTexture2D< PixelType > > ModelTexture2DParser< PixelType >::parseBinary( std::vector< char >::const_iterator& dataIt, const bool loadRecurrently, ID3D11Device3& device )
    {
	    std::shared_ptr< ModelTexture2D< PixelType > > modelTexture = std::make_shared< ModelTexture2D< PixelType > >( );

	    Texture2DFileInfo fileInfo = *Texture2DFileInfo::createFromMemory( dataIt );

        // If texture path is not set - leave texture empty.
        if ( !fileInfo.getPath().empty() )
        {
            std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, PixelType > > texture;
            if ( loadRecurrently ) 
            {
                const DXGI_FORMAT defaultFormat = ModelTexture2DParser< PixelType >::getDefaultFormat();

	            texture = std::make_shared< Texture2D< TexUsage::Default, TexBind::ShaderResource, PixelType > >
                    ( device, fileInfo, true, true, true, defaultFormat, defaultFormat );
            } else {
                texture = std::make_shared< Texture2D< TexUsage::Default, TexBind::ShaderResource, PixelType > >();
                texture->setFileInfo( fileInfo );
            }

	        modelTexture->setTexture( texture );
        }
	    modelTexture->setTexcoordIndex( BinaryFile::readInt( dataIt ) );
	    modelTexture->setColorMultiplier( BinaryFile::readFloat4( dataIt ) );

	    return modelTexture;
    }

    template< typename PixelType >
    void ModelTexture2DParser< PixelType >::writeBinary( std::vector< char >& data, const ModelTexture2D< PixelType >& modelTexture )
    {
	    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, PixelType > > texture = modelTexture.getTexture();
	
	    if ( texture ) {
		    texture->getFileInfo().saveToMemory( data );
	    } else {
		    Texture2DFileInfo emptyFileInfo;
		    emptyFileInfo.saveToMemory( data );
	    }

	    BinaryFile::writeInt( data, modelTexture.getTexcoordIndex() );
	    BinaryFile::writeFloat4( data, modelTexture.getColorMultiplier() );
    }

    
}
