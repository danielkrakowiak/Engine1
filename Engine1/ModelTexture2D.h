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

        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, PixelType > > m_texture;

        int    m_texcoordIndex;
        float4 m_colorMultiplier;
    };

    template< typename PixelType >
    ModelTexture2D< PixelType >::ModelTexture2D() : 
        m_texture( nullptr ), 
        m_texcoordIndex( 0 ), 
        m_colorMultiplier( 1.0f, 1.0f, 1.0f, 1.0f )
    {}

    template< typename PixelType >
    ModelTexture2D< PixelType >::ModelTexture2D( std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, PixelType > > texture, int texcoordIndex, float4 colorMultiplier ) : 
        m_texture( texture ), 
        m_texcoordIndex( texcoordIndex ), 
        m_colorMultiplier( colorMultiplier ) {}

    template< typename PixelType >
    ModelTexture2D< PixelType >::ModelTexture2D( const ModelTexture2D< PixelType >& obj ) :
        m_texture( obj.m_texture ),
	    m_texcoordIndex( obj.m_texcoordIndex ),
	    m_colorMultiplier( obj.m_colorMultiplier ) {}

    template< typename PixelType >
    ModelTexture2D< PixelType >::~ModelTexture2D() {}

    template< typename PixelType >
    ModelTexture2D< PixelType >& ModelTexture2D< PixelType >::operator = ( const ModelTexture2D< PixelType >& other )
    {
        m_texture         = other.m_texture;
	    m_texcoordIndex   = other.m_texcoordIndex;
	    m_colorMultiplier = other.m_colorMultiplier;

	    return *this;
    }

    template< typename PixelType >
    const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, PixelType > > ModelTexture2D< PixelType >::getTexture() const
    {
	    return m_texture;
    }

    template< typename PixelType >
    std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, PixelType > > ModelTexture2D< PixelType >::getTexture()
    {
	    return m_texture;
    }

    template< typename PixelType >
    int ModelTexture2D< PixelType >::getTexcoordIndex() const
    {
	    return m_texcoordIndex;
    }

    template< typename PixelType >
    float4 ModelTexture2D< PixelType >::getColorMultiplier() const
    {
	    return m_colorMultiplier;
    }

    template< typename PixelType >
    void ModelTexture2D< PixelType >::setTexture( std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, PixelType > > texture )
    {
	    this->m_texture = texture;
    }

    template< typename PixelType >
    void ModelTexture2D< PixelType >::setTexcoordIndex( int texcoordIndex )
    {
	    this->m_texcoordIndex = texcoordIndex;
    }

    template< typename PixelType >
    void ModelTexture2D< PixelType >::setColorMultiplier( float4 colorMultiplier )
    {
	    this->m_colorMultiplier = colorMultiplier;
    }
}