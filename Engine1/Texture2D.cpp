#include "Texture2D.h"

#include <assert.h>
#include <fstream>
#include <algorithm>

#include "StringUtil.h"

#include "BinaryFile.h"
#include "Texture2DFileInfoParser.h"

#include <d3d11.h>

using namespace Engine1;

std::shared_ptr<Texture2D> Texture2D::createFromFile( const Texture2DFileInfo& fileInfo )
{
	return createFromFile( fileInfo.getPath(), fileInfo.getFormat() );
}

std::shared_ptr<Texture2D> Texture2D::createFromFile( const std::string& path, const Texture2DFileInfo::Format format )
{
	std::shared_ptr< std::vector<char> > fileData = BinaryFile::load( path );

	std::shared_ptr<Texture2D> texture = createFromMemory( *fileData, format );

	if ( texture ) {
		texture->getFileInfo().setPath( path );
		texture->getFileInfo().setFormat( format );
	}

	return texture;
}

std::shared_ptr<Texture2D> Texture2D::createFromMemory( const std::vector<char>& fileData, const Texture2DFileInfo::Format format )
{

	std::shared_ptr<Texture2D> texture = std::make_shared<Texture2D>();

	// #TODO: FIT_BITMAP is not always the good image type (see "To do.txt").
	fipImage image( FIT_BITMAP );
	// #TODO: WARNING: casting away const qualifier on data - need to make sure that FreeImage doesn't modify the input data!
	fipMemoryIO data( const_cast<unsigned char*>( reinterpret_cast<const unsigned char*>( fileData.data( ) ) ), fileData.size( ) );

	// Parse image data.
	if ( !image.loadFromMemory( data ) )
		throw std::exception( "Texture2D::loadFromMemory - loading texture from memory failed." );

	// Convert image to 32 bpp if needed.
	if ( image.getBitsPerPixel() == 24 ) {
		if ( !image.convertTo32Bits() )
			throw std::exception( "Texture2D::loadFromMemory - loaded texture is 24 bits per pixel and had to be converted to 32 bits per pixel for compliance with Direct3D. This converions failed." );
	} else if ( image.getBitsPerPixel() != 8 ) {
		throw std::exception( "Texture2D::loadFromMemory - loaded texture is neither 24 nor 8 bits per pixel. This byte per pixel fromat is not supported." );
	}

	// Save basic info about the image.
	texture->bytesPerPixel = image.getBitsPerPixel() / 8;
	texture->width = image.getWidth();
	texture->height = image.getHeight();
	texture->size = image.getLine() * image.getHeight();

	// Create first mipmap data vector.
	texture->dataMipMaps.push_back( std::vector<unsigned char>() );
	texture->dataMipMaps.front().resize( texture->size );

	// Copy data from FIP image to data vector.
	unsigned char* srcData = static_cast<unsigned char*>( image.accessPixels() );
	std::copy( srcData, srcData + texture->size, texture->dataMipMaps.front().begin() );

	return texture;
}

Texture2D::Texture2D() :
bytesPerPixel( 0 ),
width( 0 ),
height( 0 ),
size( 0 ),
mipmapsOnGpu( false ),
dataMipMaps()
{}


Texture2D::~Texture2D()
{}

Asset::Type Texture2D::getType() const
{
	return Asset::Type::Texture2D;
}

std::vector< std::shared_ptr<const Asset> > Texture2D::getSubAssets( ) const
{
	return std::vector< std::shared_ptr<const Asset> >();
}

std::vector< std::shared_ptr<Asset> > Texture2D::getSubAssets()
{
	return std::vector< std::shared_ptr<Asset> >( );
}

void Texture2D::swapSubAsset( std::shared_ptr<Asset> oldAsset, std::shared_ptr<Asset> newAsset )
{
	throw std::exception( "Texture2D::swapSubAsset - there are no sub-assets to be swapped." );
}

void Texture2D::setFileInfo( const Texture2DFileInfo& fileInfo )
{
	this->fileInfo = fileInfo;
}

const Texture2DFileInfo& Texture2D::getFileInfo() const
{
	return fileInfo;
}

Texture2DFileInfo& Texture2D::getFileInfo()
{
	return fileInfo;
}

void Texture2D::loadCpuToGpu( ID3D11Device& device, bool reload )
{
	if ( !isInCpuMemory() ) throw std::exception( "Texture2D::loadCpuToGpu - Texture not in RAM." );

    if ( reload )
        throw std::exception( "Texture2D::loadCpuToGpu - reload not yet implemented." );

	if ( !isInGpuMemory() ) {
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory( &desc, sizeof( desc ) );
		desc.Width = getWidth();
		desc.Height = getHeight();
		desc.MipLevels = desc.ArraySize = 1; // Without mipmaps for the creation.
		unsigned int imageBitPerPixel = getBytesPerPixel() * 8;
		if ( imageBitPerPixel == 8 )  desc.Format = DXGI_FORMAT_R8_UNORM;
		if ( imageBitPerPixel == 16 ) desc.Format = DXGI_FORMAT_R8G8_UNORM;
		if ( imageBitPerPixel == 32 ) desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		else throw std::exception( "Texture2D::loadCpuToGpu - Image 'bits per pixel' format is unsupported." );
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		D3D11_SUBRESOURCE_DATA textureDataPtr;
		ZeroMemory( &textureDataPtr, sizeof( textureDataPtr ) );
		textureDataPtr.pSysMem = getData().data();
		textureDataPtr.SysMemPitch = getLineSize(); // Distance between any two adjacent pixels on different lines.
		textureDataPtr.SysMemSlicePitch = getSize(); // Size of the entire 2D surface in bytes.

		HRESULT result = device.CreateTexture2D( &desc, &textureDataPtr, texture.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "Texture2D::loadCpuToGpu - creating texture on GPU failed." );

		result = device.CreateShaderResourceView( texture.Get(), nullptr, shaderResource.ReleaseAndGetAddressOf() );
		if ( result < 0 ) throw std::exception( "Texture2D::loadCpuToGpu - creating shader resource on GPU failed." );
	} else if ( reload ) {
		throw std::exception( "Texture2D::loadCpuToGpu - re-loading texture to GPU memory not yet implemented." );
		// #TODO: upload data to the existing texture.
	}
}

void Texture2D::loadGpuToCpu()
{
	throw std::exception( "Texture2D::loadGpuToCpu - unimplemented method." );
	// #TODO: implement.
}

void Texture2D::unloadFromCpu()
{
	dataMipMaps.clear();
}

void Texture2D::unloadFromGpu()
{
	shaderResource.Reset();
	texture.Reset();

	mipmapsOnGpu = false;
}

bool Texture2D::isInCpuMemory() const
{
	// True, if there is at least one mipmap with any data.
	return !dataMipMaps.empty() && !dataMipMaps.front().empty();
}

bool Texture2D::isInGpuMemory() const
{
	// True, if texture and shader resource exist.
	return texture && shaderResource;
}

void Texture2D::generateMipMapsOnCpu()
{
	// #TODO: implement.

	if ( !isInCpuMemory() ) throw std::exception( "Texture2D::generateMipMapsOnCpu - Cannot generate, because texture is not in CPU memory." );

	throw std::exception( "Texture2D::loadGpuToCpu - unimplemented method." );
	//
	//// First, remove old mipmaps.
	//removeMipMapsOnCpu();

	//{ // Generate mip maps.
	//	int newWidth = width, newHeight = height;
	//	while ( newWidth > 1 || newHeight > 1 ) {

	//		//TODO: add support for int4 type.

	//		// Calculate new mipmap dimensions.
	//		newWidth  /= 2;
	//		newHeight /= 2;
	//		newWidth  = max( 1, newWidth );
	//		newHeight = max( 1, newHeight );

	//		// Calculate new mipmap size in bytes.
	//		int newLineSize = newWidth * bytesPerPixel;
	//		newLineSize += ( newLineSize % sizeof(WORD) ); // Round to multiple of WORD.
	//		int newSize = newLineSize * newHeight;

	//		// Get previous mipmap (as a source for the new mipmap).
	//		std::vector<unsigned char>& dataPrevMipmap = dataMipMaps.back( );

	//		// Create new mipmap data vector.
	//		dataMipMaps.push_back( std::vector<unsigned char>() );
	//		std::vector<unsigned char>& dataNextMipmap = dataMipMaps.back( );
	//		dataNextMipmap.resize( newSize );

	//		// Fill new mipmap.
	//		for ( int y = 0; y < newHeight; ++y ) {
	//			for ( int x = 0; x < newWidth; ++x ) {
	//				int prevIndex = 
	//				int nextIndex = ( y * newLineSize + x ) * bytesPerPixel;
	//				dataNextMipmap.at( i )

	//					dest_red = ( dataPrevMipmap.at()[ i * 2 ][ j * 2 ] + source->red[ i * 2 ][ j * 2 + 1 ]
	//					+ source->red[ i * 2 + 1 ][ j * 2 ] + source->red[ i * 2 + 1 ][ j * 2 + 1 ] ) / 4;
	//				dest_green = ( source->green[ i * 2 ][ j * 2 ] + source->green[ i * 2 ][ j * 2 + 1 ]
	//							   + source->green[ i * 2 + 1 ][ j * 2 ] + source->green[ i * 2 + 1 ][ j * 2 + 1 ] ) / 4;
	//				dest_blue = ( source->blue[ i * 2 ][ j * 2 ] + source->blue[ i * 2 ][ j * 2 + 1 ]
	//							  + source->blue[ i * 2 + 1 ][ j * 2 ] + source->blue[ i * 2 + 1 ][ j * 2 + 1 ] ) / 4;
	//			}
	//		}

	//		//TODO: calculate next mipmap manually or through FreeImage?

	//		imageMipMaps.push_back( std::make_unique<fipImage>( (FREE_IMAGE_TYPE)prevMipMap.getImageType() ) );
	//		fipImage& newMipMap = *imageMipMaps.back();

	//		prevMipMap.copySubImage( newMipMap, 0, 0, prevMipMap.getWidth(), prevMipMap.getHeight() );
	//		newMipMapWidth = prevMipMap.getWidth() / 2, newMipMapHeight = prevMipMap.getHeight() / 2;
	//		if ( newMipMapWidth == 0 ) newMipMapWidth = 1;
	//		if ( newMipMapHeight == 0 ) newMipMapHeight = 1;

	//		newMipMap.rescale( newMipMapWidth, newMipMapHeight, FILTER_BILINEAR );
	//	} while ( newMipMapWidth > 1 || newMipMapHeight > 1 );

	//	mipMaps = true;
	//}
}

void Texture2D::generateMipMapsOnGpu( ID3D11DeviceContext& deviceContext )
{
	if ( !isInGpuMemory() ) throw std::exception( "Texture2D::generateMipMapsOnGpu - Cannot generate, because texture is not in GPU memory." );

	deviceContext.GenerateMips( shaderResource.Get() );

	mipmapsOnGpu = true;
}

void Texture2D::removeMipMapsOnCpu()
{
	if ( !isInCpuMemory() ) throw std::exception( "Texture2D::removeMipMapsOnCpu - Cannot remove, because texture is not in CPU memory." );

	// Remove mipmaps until only 1 is left.
	while ( dataMipMaps.size() > 1 )
		dataMipMaps.pop_back();
}

bool Texture2D::hasMipMapsOnCpu()      const
{
	// True, if there is more than one mipmap.
	return dataMipMaps.size() > 1;
}

bool Texture2D::hasMipMapsOnGpu()     const
{
	return mipmapsOnGpu;
}

int  Texture2D::getMipMapCountOnCpu()  const
{
	return dataMipMaps.size();
}

int  Texture2D::getMipMapCountOnGpu() const
{
	if ( isInGpuMemory() ) {
		if ( hasMipMapsOnGpu() )
			// Has all the mipmaps until 1x1.
			return 1 + (int)( floor( log2( std::max( width, height ) ) ) );
		else
			// Has only main image.
			return 1;
	} else {
		// Not in GPU memory.
		return 0;
	}
}

std::vector<unsigned char>& Texture2D::getData( unsigned int mipMapLevel )
{
	// Call const version of the same method and cast aways const.
	return const_cast<std::vector<unsigned char>&>( static_cast<const Texture2D*>( this )->getData( mipMapLevel ) );
}

const std::vector<unsigned char>& Texture2D::getData( unsigned int mipMapLevel ) const
{
	if ( mipMapLevel >= dataMipMaps.size() ) throw std::exception( "Texture2D::getData - Incorrect level requested. There is no mipmap with such level." );

	return dataMipMaps.at( mipMapLevel );
}

ID3D11Texture2D* Texture2D::getTexture() const
{
	if ( !isInGpuMemory() ) throw std::exception( "Texture2D::getTexture - Failed, because texture is not in GPU memory." );

	return texture.Get();
}

ID3D11ShaderResourceView* Texture2D::getShaderResource() const
{
	if ( !isInGpuMemory() ) throw std::exception( "Texture2D::getTexture - Failed, because texture is not in GPU memory." );

	return shaderResource.Get();
}

int Texture2D::getBytesPerPixel() const
{
	return bytesPerPixel;
}

int Texture2D::getWidth( unsigned int mipMapLevel ) const
{
	if ( (int)mipMapLevel >= getMipMapCountOnCpu() && (int)mipMapLevel >= getMipMapCountOnGpu() )
		throw std::exception( "Texture2D::getWidth - Incorrect level requested. There is no mipmap with such level." );

	return std::max( 1, width / ( 1 + (int)mipMapLevel ) );
}

int Texture2D::getHeight( unsigned int mipMapLevel ) const
{
	if ( (int)mipMapLevel >= getMipMapCountOnCpu() && (int)mipMapLevel >= getMipMapCountOnGpu() )
		throw std::exception( "Texture2D::getHeight - Incorrect level requested. There is no mipmap with such level." );

	return std::max( 1, height / ( 1 + (int)mipMapLevel ) );
}

int Texture2D::getSize( unsigned int mipMapLevel ) const
{
	if ( (int)mipMapLevel >= getMipMapCountOnCpu() && (int)mipMapLevel >= getMipMapCountOnGpu() )
		throw std::exception( "Texture2D::getSize - Incorrect level requested. There is no mipmap with such level." );

	return getLineSize( mipMapLevel ) * getHeight( mipMapLevel );
}

int Texture2D::getLineSize( unsigned int mipMapLevel ) const
{
	if ( (int)mipMapLevel >= getMipMapCountOnCpu() && (int)mipMapLevel >= getMipMapCountOnGpu() )
		throw std::exception( "Texture2D::getLineSize - Incorrect level requested. There is no mipmap with such level." );

	const int lineSize = getWidth( mipMapLevel ) * bytesPerPixel;
	// Round to sizeof WORD.
	const int lineSizePadded = lineSize + ( lineSize % sizeof( WORD ) );

	return lineSizePadded;
}
