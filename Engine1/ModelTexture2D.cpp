#include "ModelTexture2D.h"


ModelTexture2D::ModelTexture2D( std::shared_ptr<Texture2D>& texture, int texcoordIndex, float4 colorMultiplier ) 
: texture( texture ), texcoordIndex( texcoordIndex ), colorMultiplier( colorMultiplier ) {}

ModelTexture2D::ModelTexture2D( const ModelTexture2D& obj ) :
	texture( obj.texture ), 
	texcoordIndex( obj.texcoordIndex ), 
	colorMultiplier( obj.colorMultiplier ) {}

ModelTexture2D::~ModelTexture2D() {}
