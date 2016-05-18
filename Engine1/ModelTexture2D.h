#pragma once

#include <memory>

#include "float4.h"
#include "uchar4.h"
#include "TTexture2D.h"

namespace Engine1
{
    template< typename PixelType >
    class ModelTexture2D
    {
        public:

        ModelTexture2D();
        ModelTexture2D( std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, PixelType > > texture, int texcoordIndex = 0, float4 colorMultiplier = float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
        ModelTexture2D( const ModelTexture2D< PixelType >& );
        ~ModelTexture2D();

        ModelTexture2D< PixelType >& operator=(const ModelTexture2D< PixelType >&);

        const std::shared_ptr< Texture2DSpecBind<TexBind::ShaderResource, PixelType > > getTexture() const;
        /* */ std::shared_ptr< Texture2DSpecBind<TexBind::ShaderResource, PixelType > > getTexture();

        int    getTexcoordIndex()   const;
        float4 getColorMultiplier() const;

        void setTexture( std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, PixelType > > texture );
        void setTexcoordIndex( int texcoordIndex );
        void setColorMultiplier( float4 colorMultiplier );

        private:

        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, PixelType > > texture;

        int    texcoordIndex;
        float4 colorMultiplier;
    };

    template< typename PixelType >
    ModelTexture2D< PixelType >::ModelTexture2D()
    : texture( nullptr ), texcoordIndex( 0 ), colorMultiplier( 1.0f, 1.0f, 1.0f, 1.0f )
    {}

    template< typename PixelType >
    ModelTexture2D< PixelType >::ModelTexture2D( std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, PixelType > > texture, int texcoordIndex, float4 colorMultiplier ) 
    : texture( texture ), texcoordIndex( texcoordIndex ), colorMultiplier( colorMultiplier ) {}

    template< typename PixelType >
    ModelTexture2D< PixelType >::ModelTexture2D( const ModelTexture2D< PixelType >& obj ) :
	    texture( obj.texture ), 
	    texcoordIndex( obj.texcoordIndex ), 
	    colorMultiplier( obj.colorMultiplier ) {}

    template< typename PixelType >
    ModelTexture2D< PixelType >::~ModelTexture2D() {}

    template< typename PixelType >
    ModelTexture2D< PixelType >& ModelTexture2D< PixelType >::operator = ( const ModelTexture2D< PixelType >& other )
    {
	    texture         = other.texture;
	    texcoordIndex   = other.texcoordIndex;
	    colorMultiplier = other.colorMultiplier;

	    return *this;
    }

    template< typename PixelType >
    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, PixelType > > ModelTexture2D< PixelType >::getTexture() const
    {
	    return texture;
    }

    template< typename PixelType >
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, PixelType > > ModelTexture2D< PixelType >::getTexture()
    {
	    return texture;
    }

    template< typename PixelType >
    int ModelTexture2D< PixelType >::getTexcoordIndex() const
    {
	    return texcoordIndex;
    }

    template< typename PixelType >
    float4 ModelTexture2D< PixelType >::getColorMultiplier() const
    {
	    return colorMultiplier;
    }

    template< typename PixelType >
    void ModelTexture2D< PixelType >::setTexture( std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, PixelType > > texture )
    {
	    this->texture = texture;
    }

    template< typename PixelType >
    void ModelTexture2D< PixelType >::setTexcoordIndex( int texcoordIndex )
    {
	    this->texcoordIndex = texcoordIndex;
    }

    template< typename PixelType >
    void ModelTexture2D< PixelType >::setColorMultiplier( float4 colorMultiplier )
    {
	    this->colorMultiplier = colorMultiplier;
    }
}