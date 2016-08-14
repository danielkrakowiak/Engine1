#include "BlockActor.h"

using namespace Engine1;

BlockActor::BlockActor( std::shared_ptr<BlockModel> model, const float43& pose ) :
    m_model( model ),
    m_pose( pose )
{}


BlockActor::~BlockActor()
{}

Actor::Type BlockActor::getType() const
{
    return Type::BlockActor;
}

const float43& BlockActor::getPose() const
{
    return m_pose;
}

float43& BlockActor::getPose( )
{
    return m_pose;
}

std::shared_ptr<const BlockModel> BlockActor::getModel( ) const
{
    return m_model;
}

std::shared_ptr<BlockModel> BlockActor::getModel( )
{
    return m_model;
}

void BlockActor::setPose( const float43& pose )
{
    this->m_pose = pose;
}

void BlockActor::setModel( std::shared_ptr<BlockModel> model )
{
    this->m_model = model;
}
