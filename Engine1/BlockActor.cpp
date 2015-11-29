#include "BlockActor.h"


BlockActor::BlockActor( std::shared_ptr<BlockModel> model, const float43& pose ) :
    model( model ),
    pose( pose )
{}


BlockActor::~BlockActor()
{}

Actor::Type BlockActor::getType() const
{
    return Type::BlockActor;
}

const float43& BlockActor::getPose() const
{
    return pose;
}

float43& BlockActor::getPose( )
{
    return pose;
}

std::shared_ptr<const BlockModel> BlockActor::getModel( ) const
{
    return model;
}

std::shared_ptr<BlockModel> BlockActor::getModel( )
{
    return model;
}

void BlockActor::setPose( const float43& pose )
{
    this->pose = pose;
}

void BlockActor::setModel( std::shared_ptr<BlockModel> model )
{
    this->model = model;
}
