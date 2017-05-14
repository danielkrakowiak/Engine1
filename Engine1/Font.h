#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <string>
#include <array>
#include <map>
#include <wrl.h>

#include "float2.h"
#include "float3.h"
#include "int2.h"
#include "uint2.h"
#include "uint3.h"

#include <d3d11.h>

namespace Engine1
{
    class FontCharacter
    {
        friend class Font;
        public:

        static ID3D11Buffer* getDefaultTexcoordsBuffer() { return defaultTexcoordsBuffer.Get(); }
        static ID3D11Buffer* getDefaultTriangleBuffer() { return defaultTriangleBuffer.Get(); }

        FontCharacter( unsigned long charcode, int2 pos, int2 advance, uint2 size ) :
            m_charcode( charcode ),
            m_pos( pos ),
            m_advance( advance ),
            m_size( size ),
            m_vertexBuffer( nullptr ),
            m_texture( nullptr ),
            m_textureResource( nullptr )
        {}

        FontCharacter( const FontCharacter& obj ) :
            m_charcode( obj.m_charcode ),
            m_pos( obj.m_pos ),
            m_advance( obj.m_advance ),
            m_size( obj.m_size ),
            m_vertexBuffer( obj.m_vertexBuffer ),
            m_texture( obj.m_texture ),
            m_textureResource( obj.m_textureResource )
        {}

        ~FontCharacter()
        {}

        bool operator() ( const FontCharacter& lhs, const FontCharacter& rhs ) const { return lhs.m_charcode < rhs.m_charcode; }

        unsigned long getCharcode() const { return m_charcode; }
        int2          getPos()      const { return m_pos; }
        int2          getAdvance()  const { return m_advance; }
        uint2         getSize()     const { return m_size; }

        ID3D11Buffer*             getVertexBuffer() const { return m_vertexBuffer.Get(); }
        ID3D11Texture2D*          getTexture() const { return m_texture.Get(); };
        ID3D11ShaderResourceView* getTextureResource() const { return m_textureResource.Get(); };

        private:
        static void setDefaultTexcoordsBuffer( ID3D11Buffer& texcoordsBuffer ) { defaultTexcoordsBuffer = &texcoordsBuffer; }
        static void setDefaultTriangleBuffer( ID3D11Buffer& triangleBuffer ) { defaultTriangleBuffer = &triangleBuffer; }

        static Microsoft::WRL::ComPtr<ID3D11Buffer> defaultTexcoordsBuffer;
        static Microsoft::WRL::ComPtr<ID3D11Buffer> defaultTriangleBuffer;


        void setVertexBuffer( ID3D11Buffer& vertexBuffer ) { this->m_vertexBuffer = &vertexBuffer; }
        void setTexture( ID3D11Texture2D& texture, ID3D11ShaderResourceView& textureResource ) { this->m_texture = &texture; this->m_textureResource = &textureResource; };

        unsigned long m_charcode;
        int2          m_pos;
        int2          m_advance;
        uint2         m_size;

        Microsoft::WRL::ComPtr<ID3D11Buffer>             m_vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Texture2D>          m_texture;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_textureResource;

        // Copying is not allowed.
        FontCharacter& operator=(const FontCharacter&) = delete;
    };

    class Font
    {
        public:
        Font( int2 targetScreenSize ) : 
            m_targetScreenSize( targetScreenSize ), 
            m_face( nullptr ), 
            m_loaded( false ), 
            m_path( "" ), 
            m_size( 0 ) {}

        Font( int2 targetScreenSize, std::string path, unsigned int size ) :
            m_targetScreenSize( targetScreenSize ),
            m_face( nullptr ),
            m_loaded( false ),
            m_path( "" ),
            m_size( 0 )
        {
            loadFromFile( path, size );
        }
        ~Font( void ) {}

        void loadFromFile( std::string path, unsigned int size );
        FontCharacter* getCharacter( unsigned long charcode, ID3D11Device& device );
        int getLineHeight() const;

        private:

        bool  m_loaded;
        int2 m_targetScreenSize;

        FT_Face      m_face;
        std::string  m_path;
        unsigned int m_size;

        std::map<unsigned long, FontCharacter> m_characters;

        // Copying is not allowed.
        Font( const Font& ) = delete;
        Font& operator=(const Font&) = delete;

    };
}