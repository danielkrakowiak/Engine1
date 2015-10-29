#include "Font.h"

#include "FontLibrary.h"

#include "Direct3DUtil.h"

using Microsoft::WRL::ComPtr;

ComPtr<ID3D11Buffer> FontCharacter::defaultTexcoordsBuffer = nullptr;
ComPtr<ID3D11Buffer> FontCharacter::defaultTriangleBuffer  = nullptr;

void Font::loadFromFile( std::string path, unsigned int size ) {
	if ( FT_New_Face( FontLibrary::get(), path.c_str(), 0, &face ) ) throw std::exception( "Font::loadFromFile - loading font failed" );

	//TODO: fixed device resoulution? Probably shouldn't be left like that?
	FT_Set_Char_Size(
		face,    /* handle to face object           */
		0,       /* char_width in 1/64th of points  */
		size * 64,   /* char_height in 1/64th of points */
		72,     /* horizontal device resolution    */ 
		72   /* vertical device resolution      */
		);

	this->path = path;
	this->size = size;

	loaded = true;
}

FontCharacter* Font::getCharacter( unsigned long charcode, ID3D11Device& device, ID3D11DeviceContext& deviceContext ) {
	if ( !loaded ) return 0;

	std::map<unsigned long, FontCharacter>::iterator charactersIt;

	charactersIt = characters.find( charcode );
	if ( charactersIt != characters.end() ) {
		//character was found
		return &( *charactersIt ).second;
	} else {
		//character wasn't found
		if ( !FT_Load_Char( face, charcode, FT_LOAD_RENDER ) ) {
			//character with given charcode is available in the font
			FontCharacter character( charcode, int2( face->glyph->bitmap_left, face->glyph->bitmap_top ),
									 int2( face->glyph->advance.x / 64, face->glyph->advance.y / 64 ),
									 uint2( face->glyph->bitmap.width, face->glyph->bitmap.rows )
									 ); //advance is in 1/64 pixel units

			{ //create geometry for the character
				{ //create vertices
					float pixelWidth = 2.0f / (float)targetScreenSize.x;
					float pixelHeight = 2.0f / (float)targetScreenSize.y;

					float characterWidth = (float)character.getSize().x;
					float characterHeight = (float)character.getSize( ).y;

					std::array<float3, 4> vertices;

					vertices[ 0 ] = float3( 0.0f, 0.0f, 0.5f ); //clockwise order
					vertices[ 1 ] = float3( characterWidth, 0.0f, 0.5f );
					vertices[ 2 ] = float3( characterWidth, -characterHeight, 0.5f );
					vertices[ 3 ] = float3( 0.0f, -characterHeight, 0.5f ); 
					
					D3D11_BUFFER_DESC vertexBufferDesc;
					ZeroMemory( &vertexBufferDesc, sizeof( vertexBufferDesc ) );

					vertexBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
					vertexBufferDesc.ByteWidth           = sizeof(float3)* vertices.size();
					vertexBufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
					vertexBufferDesc.CPUAccessFlags      = 0;
					vertexBufferDesc.MiscFlags           = 0;
					vertexBufferDesc.StructureByteStride = 0;

					D3D11_SUBRESOURCE_DATA vertexDataPtr;
					ZeroMemory( &vertexDataPtr, sizeof( vertexDataPtr ) );

					vertexDataPtr.pSysMem          = vertices.data();
					vertexDataPtr.SysMemPitch      = 0;
					vertexDataPtr.SysMemSlicePitch = 0;

					ComPtr<ID3D11Buffer> vertexBuffer;

					HRESULT result = device.CreateBuffer( &vertexBufferDesc, &vertexDataPtr, vertexBuffer.ReleaseAndGetAddressOf() );
					if ( result < 0 ) throw std::exception( "Font::getCharacter - loading character vertices to GPU failed" );

					#if defined(DEBUG_DIRECT3D) || defined(_DEBUG) 
					std::string resourceName = std::string( "FontCharacter::vertexBuffer - charcode: " ) + std::to_string( charcode );
					Direct3DUtil::setResourceName( *vertexBuffer.Get(), resourceName );
					#endif

					character.setVertexBuffer( *vertexBuffer.Get() );
				}

				{ //create texcoords
					//create default texcoords if not created yet
					if ( !FontCharacter::getDefaultTexcoordsBuffer() ) {

						std::array<float2, 4> texcoords;

						texcoords[ 0 ] = float2( 0.0f, 0.0f );
						texcoords[ 1 ] = float2( 1.0f, 0.0f );
						texcoords[ 2 ] = float2( 1.0f, 1.0f );
						texcoords[ 3 ] = float2( 0.0f, 1.0f );

						D3D11_BUFFER_DESC texcoordBufferDesc;
						ZeroMemory( &texcoordBufferDesc, sizeof( texcoordBufferDesc ) );

						texcoordBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
						texcoordBufferDesc.ByteWidth           = sizeof(float2)* texcoords.size( );
						texcoordBufferDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
						texcoordBufferDesc.CPUAccessFlags      = 0;
						texcoordBufferDesc.MiscFlags           = 0;
						texcoordBufferDesc.StructureByteStride = 0;

						D3D11_SUBRESOURCE_DATA texcoordDataPtr;
						ZeroMemory( &texcoordDataPtr, sizeof( texcoordDataPtr ) );

						texcoordDataPtr.pSysMem          = texcoords.data( );
						texcoordDataPtr.SysMemPitch      = 0;
						texcoordDataPtr.SysMemSlicePitch = 0;

						ComPtr<ID3D11Buffer> buffer;

						HRESULT result = device.CreateBuffer( &texcoordBufferDesc, &texcoordDataPtr, buffer.ReleaseAndGetAddressOf() );
						if ( result < 0 ) throw std::exception( "Font::getCharacter - loading character texcoords to GPU failed" );

						FontCharacter::setDefaultTexcoordsBuffer( *buffer.Get() );
					}
				}

				{ //create triangles
					//create default triangles if not created yet
					if ( !FontCharacter::getDefaultTriangleBuffer( ) ) {

						std::array<uint3, 2> triangles;

						triangles[ 0 ] = uint3( 0, 1, 2 ); //clockwise order
						triangles[ 1 ] = uint3( 0, 2, 3 );

						D3D11_BUFFER_DESC triangleBufferDesc;
						ZeroMemory( &triangleBufferDesc, sizeof( triangleBufferDesc ) );

						triangleBufferDesc.Usage               = D3D11_USAGE_DEFAULT;
						triangleBufferDesc.ByteWidth           = sizeof(uint3)* triangles.size( );
						triangleBufferDesc.BindFlags           = D3D11_BIND_INDEX_BUFFER;
						triangleBufferDesc.CPUAccessFlags      = 0;
						triangleBufferDesc.MiscFlags           = 0;
						triangleBufferDesc.StructureByteStride = 0;

						D3D11_SUBRESOURCE_DATA triangleDataPtr;
						ZeroMemory( &triangleDataPtr, sizeof( triangleDataPtr ) );

						triangleDataPtr.pSysMem          = triangles.data( );
						triangleDataPtr.SysMemPitch      = 0;
						triangleDataPtr.SysMemSlicePitch = 0;

						ComPtr<ID3D11Buffer> buffer;

						HRESULT result = device.CreateBuffer( &triangleBufferDesc, &triangleDataPtr, buffer.ReleaseAndGetAddressOf() );
						if ( result < 0 ) throw std::exception( "Font::getCharacter - loading character triangles to GPU failed" );

						FontCharacter::setDefaultTriangleBuffer( *buffer.Get() );
					}
				}
			}

			{ //create texture for the character
				if ( face->glyph->bitmap.buffer ) { //if character has bitmap (ex: space doesn't have)
					D3D11_TEXTURE2D_DESC desc;
					ZeroMemory( &desc, sizeof( desc ) );

					desc.Width              = character.getSize().x;
					desc.Height             = character.getSize().y;
					desc.MipLevels          = desc.ArraySize = 1; //without mipmaps
					desc.Format             = DXGI_FORMAT_R8_UNORM;
					desc.SampleDesc.Count   = 1;
					desc.SampleDesc.Quality = 0;
					desc.Usage              = D3D11_USAGE_DEFAULT;
					desc.BindFlags          = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
					desc.CPUAccessFlags     = 0;
					desc.MiscFlags          = 0;

					D3D11_SUBRESOURCE_DATA textureDataPtr;
					ZeroMemory( &textureDataPtr, sizeof( textureDataPtr ) );

					textureDataPtr.pSysMem = face->glyph->bitmap.buffer;
					textureDataPtr.SysMemPitch = character.getSize().x; //distance between any two adjacent pixels on different lines
					textureDataPtr.SysMemSlicePitch = character.getSize().x * character.getSize().y; //size of the entire 2D surface in bytes

					ComPtr<ID3D11Texture2D>          texture;
					ComPtr<ID3D11ShaderResourceView> textureResource;

					HRESULT result = device.CreateTexture2D( &desc, &textureDataPtr, texture.ReleaseAndGetAddressOf() );
					if ( result < 0 ) throw std::exception( "Font::getCharacter - loading character bitmap to GPU failed" );

					result = device.CreateShaderResourceView( texture.Get(), nullptr, textureResource.ReleaseAndGetAddressOf() );
					if ( result < 0 ) throw std::exception( "Font::getCharacter - loading character bitmap to GPU failed" );

					character.setTexture( *texture.Get(), *textureResource.Get() );
				}
			}

			characters.insert( std::pair<unsigned int, FontCharacter>( charcode, character ) );
		} else {
			//character with given charcode is not available in the font
			return nullptr;
		}

		return &( *( characters.find( charcode ) ) ).second;
	}
}