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

class FontCharacter
{
	friend class Font;
	public:

	static ID3D11Buffer* getDefaultTexcoordsBuffer() { return defaultTexcoordsBuffer.Get(); }
	static ID3D11Buffer* getDefaultTriangleBuffer() { return defaultTriangleBuffer.Get(); }

	FontCharacter( unsigned long charcode, int2 pos, int2 advance, uint2 size ) :
		charcode( charcode ),
		pos( pos ),
		advance( advance ),
		size( size ),
		vertexBuffer( nullptr ),
		texture( nullptr ),
		textureResource( nullptr )
	{}

	FontCharacter( const FontCharacter& obj ) :
		charcode( obj.charcode ),
		pos( obj.pos ),
		advance( obj.advance ),
		size( obj.size ),
		vertexBuffer( obj.vertexBuffer ),
		texture( obj.texture ),
		textureResource( obj.textureResource )
	{}

	~FontCharacter()
	{
	}

	bool operator() ( const FontCharacter& lhs, const FontCharacter& rhs ) const { return lhs.charcode < rhs.charcode; }

	unsigned long getCharcode() const { return charcode; }
	int2          getPos()      const { return pos; }
	int2          getAdvance()  const { return advance; }
	uint2         getSize()     const { return size; }

	ID3D11Buffer*             getVertexBuffer() const { return vertexBuffer.Get(); }
	ID3D11Texture2D*          getTexture() const { return texture.Get(); };
	ID3D11ShaderResourceView* getTextureResource() const { return textureResource.Get(); };

	private:
	static void setDefaultTexcoordsBuffer( ID3D11Buffer& texcoordsBuffer ) { defaultTexcoordsBuffer = &texcoordsBuffer; }
	static void setDefaultTriangleBuffer( ID3D11Buffer& triangleBuffer ) { defaultTriangleBuffer = &triangleBuffer; }

	static Microsoft::WRL::ComPtr<ID3D11Buffer> defaultTexcoordsBuffer;
	static Microsoft::WRL::ComPtr<ID3D11Buffer> defaultTriangleBuffer;


	void setVertexBuffer( ID3D11Buffer& vertexBuffer ) { this->vertexBuffer = &vertexBuffer; }
	void setTexture( ID3D11Texture2D& texture, ID3D11ShaderResourceView& textureResource ) { this->texture = &texture; this->textureResource = &textureResource; };

	unsigned long charcode;
	int2          pos;
	int2          advance;
	uint2         size;

	Microsoft::WRL::ComPtr<ID3D11Buffer>             vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>          texture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureResource;

	// Copying is not allowed.
	FontCharacter& operator=( const FontCharacter& ) = delete;
};

class Font
{
	public:
	Font( uint2 targetScreenSize ) : targetScreenSize( targetScreenSize ), face( nullptr ), loaded( false ), path( "" ), size( 0 ) {}
	Font( uint2 targetScreenSize, std::string path, unsigned int size ) :
		targetScreenSize( targetScreenSize ),
		face( nullptr ),
		loaded( false ),
		path( "" ),
		size( 0 )
	{
		loadFromFile( path, size );
	}
	~Font( void ) {}

	void loadFromFile( std::string path, unsigned int size );
	FontCharacter* getCharacter( unsigned long charcode, ID3D11Device& device, ID3D11DeviceContext& deviceContext );

	private:

	bool  loaded;
	uint2 targetScreenSize;

	FT_Face      face;
	std::string  path;
	unsigned int size;

	std::map<unsigned long, FontCharacter> characters;

	// Copying is not allowed.
	Font( const Font& ) = delete;
	Font& operator=( const Font& ) = delete;

};