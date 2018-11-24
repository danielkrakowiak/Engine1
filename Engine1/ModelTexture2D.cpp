//#include "ModelTexture2D.h"
//
//#include "ModelTexture2DParser.h"
//
//#include "uchar4.h"
//
//using namespace Engine1;
//
//template< typename PixelType >
//std::shared_ptr< ModelTexture2D< PixelType > > ModelTexture2D< PixelType >::createFromMemory( std::vector<char>::const_iterator& dataIt, const bool loadRecurrently, ID3D11Device3& device )
//{
//	return ModelTexture2DParser::parseBinary( dataIt, loadRecurrently, device );
//}
//
//template< typename PixelType >
//ModelTexture2D< PixelType >::ModelTexture2D()
//: texture( nullptr ), texcoordIndex( 0 ), colorMultiplier( 1.0f, 1.0f, 1.0f, 1.0f )
//{}
//
//template< typename PixelType >
//ModelTexture2D< PixelType >::ModelTexture2D( std::shared_ptr< Texture2D< PixelType > > texture, int texcoordIndex, float4 colorMultiplier ) 
//: texture( texture ), texcoordIndex( texcoordIndex ), colorMultiplier( colorMultiplier ) {}
//
//template< typename PixelType >
//ModelTexture2D< PixelType >::ModelTexture2D( const ModelTexture2D< PixelType >& obj ) :
//	texture( obj.texture ), 
//	texcoordIndex( obj.texcoordIndex ), 
//	colorMultiplier( obj.colorMultiplier ) {}
//
//template< typename PixelType >
//ModelTexture2D< PixelType >::~ModelTexture2D() {}
//
//template< typename PixelType >
//ModelTexture2D< PixelType >& ModelTexture2D< PixelType >::operator = ( const ModelTexture2D< PixelType >& other )
//{
//	texture         = other.texture;
//	texcoordIndex   = other.texcoordIndex;
//	colorMultiplier = other.colorMultiplier;
//
//	return *this;
//}
//
//template< typename PixelType >
//void ModelTexture2D< PixelType >::saveToMemory( std::vector<char>& data ) const
//{
//    ModelTexture2DParser::writeBinary( data, *this );
//}
//
//template< typename PixelType >
//const std::shared_ptr< Texture2D< PixelType > > ModelTexture2D< PixelType >::getTexture() const
//{
//	return texture;
//}
//
//template< typename PixelType >
//std::shared_ptr< Texture2D< PixelType > > ModelTexture2D< PixelType >::getTexture()
//{
//	return texture;
//}
//
//template< typename PixelType >
//int ModelTexture2D< PixelType >::getTexcoordIndex() const
//{
//	return texcoordIndex;
//}
//
//template< typename PixelType >
//float4 ModelTexture2D< PixelType >::getColorMultiplier() const
//{
//	return colorMultiplier;
//}
//
//template< typename PixelType >
//void ModelTexture2D< PixelType >::setTexture( std::shared_ptr< Texture2D< PixelType > > texture )
//{
//	this->texture = texture;
//}
//
//template< typename PixelType >
//void ModelTexture2D< PixelType >::setTexcoordIndex( int texcoordIndex )
//{
//	this->texcoordIndex = texcoordIndex;
//}
//
//template< typename PixelType >
//void ModelTexture2D< PixelType >::setColorMultiplier( float4 colorMultiplier )
//{
//	this->colorMultiplier = colorMultiplier;
//}
