#include "ModelTexture2D.h"

#include "ModelTexture2DParser.h"

#include "uchar4.h"

using namespace Engine1;

std::shared_ptr< ModelTexture2D > ModelTexture2D::createFromMemory( std::vector<char>::const_iterator& dataIt, const bool loadRecurrently, ID3D11Device& device )
{
	return ModelTexture2DParser::parseBinary( dataIt, loadRecurrently, device );
}

ModelTexture2D::ModelTexture2D()
: texture( nullptr ), texcoordIndex( 0 ), colorMultiplier( 1.0f, 1.0f, 1.0f, 1.0f )
{}

ModelTexture2D::ModelTexture2D( std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > texture, int texcoordIndex, float4 colorMultiplier ) 
: texture( texture ), texcoordIndex( texcoordIndex ), colorMultiplier( colorMultiplier ) {}

ModelTexture2D::ModelTexture2D( const ModelTexture2D& obj ) :
	texture( obj.texture ), 
	texcoordIndex( obj.texcoordIndex ), 
	colorMultiplier( obj.colorMultiplier ) {}

ModelTexture2D::~ModelTexture2D() {}

ModelTexture2D& ModelTexture2D::operator = ( const ModelTexture2D& other )
{
	texture         = other.texture;
	texcoordIndex   = other.texcoordIndex;
	colorMultiplier = other.colorMultiplier;

	return *this;
}

void ModelTexture2D::saveToMemory( std::vector<char>& data ) const
{
    ModelTexture2DParser::writeBinary( data, *this );
}

const std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > ModelTexture2D::getTexture() const
{
	return texture;
}

std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > ModelTexture2D::getTexture()
{
	return texture;
}

int ModelTexture2D::getTexcoordIndex() const
{
	return texcoordIndex;
}

float4 ModelTexture2D::getColorMultiplier() const
{
	return colorMultiplier;
}

void ModelTexture2D::setTexture( std::shared_ptr< Texture2DSpecBind< TexBind::ShaderResource, uchar4 > > texture )
{
	this->texture = texture;
}

void ModelTexture2D::setTexcoordIndex( int texcoordIndex )
{
	this->texcoordIndex = texcoordIndex;
}

void ModelTexture2D::setColorMultiplier( float4 colorMultiplier )
{
	this->colorMultiplier = colorMultiplier;
}
