#pragma once

#include <memory>

#include "float4.h"
#include "uchar4.h"
#include "TTexture2D.h"

namespace Engine1
{
    class ModelTexture2D
    {

        public:

        static std::shared_ptr< ModelTexture2D > createFromMemory( std::vector<char>::const_iterator& dataIt, const bool loadRecurrently, ID3D11Device& device );

        ModelTexture2D();
        ModelTexture2D( std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > texture, int texcoordIndex = 0, float4 colorMultiplier = float4( 1.0f, 1.0f, 1.0f, 1.0f ) );
        ModelTexture2D( const ModelTexture2D& );
        ~ModelTexture2D();

        ModelTexture2D& operator=(const ModelTexture2D&);

        void saveToMemory( std::vector<char>& data ) const;

        const std::shared_ptr< Texture2DSpecBind<TexBind::ShaderResource, uchar4> > getTexture() const;
        /* */ std::shared_ptr< Texture2DSpecBind<TexBind::ShaderResource, uchar4> > getTexture();

        int    getTexcoordIndex()   const;
        float4 getColorMultiplier() const;

        void setTexture( std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > texture );
        void setTexcoordIndex( int texcoordIndex );
        void setColorMultiplier( float4 colorMultiplier );

        private:

        std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > texture;

        int    texcoordIndex;
        float4 colorMultiplier;


    };
}

