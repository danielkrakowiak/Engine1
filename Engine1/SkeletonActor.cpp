#include "SkeletonActor.h"

#include "SkeletonModel.h"

using namespace Engine1;

SkeletonActor::SkeletonActor( std::shared_ptr<SkeletonModel> model, const float43& pose ) :
    pose( pose ),
    model( model )
{
    resetSkeletonPose();
}

SkeletonActor::SkeletonActor( std::shared_ptr<SkeletonModel> model, const float43& pose, const SkeletonPose& skeletonPose ) :
    pose( pose ),
    skeletonPose( skeletonPose ),
    model( model )
{
    //#TODO: should it check whether skeletonPose is correct for the passed mesh?
}

SkeletonActor::~SkeletonActor( )
{

}

Actor::Type SkeletonActor::getType() const
{
    return Type::SkeletonActor;
}

const float43& SkeletonActor::getPose( ) const
{
    return pose;
}

float43& SkeletonActor::getPose( )
{
    return pose;
}

const SkeletonPose& SkeletonActor::getSkeletonPose( ) const
{
    return skeletonPose;
}

SkeletonPose& SkeletonActor::getSkeletonPose( )
{
    return skeletonPose;
}

std::shared_ptr<const SkeletonModel> SkeletonActor::getModel( ) const
{
    return model;
}

std::shared_ptr<SkeletonModel> SkeletonActor::getModel( )
{
    return model;
}

void SkeletonActor::setPose( const float43& pose )
{
    this->pose = pose;
}

void SkeletonActor::setSkeletonPose( const SkeletonPose& poseInSkeletonSpace )
{
    this->skeletonPose = poseInSkeletonSpace;
    //#TODO: should it check whether skeletonPose is correct for the passed mesh?
}

void SkeletonActor::setModel( std::shared_ptr<SkeletonModel> model )
{
    this->model = model;
    
    //#TODO: should it check whether skeletonPose is correct for the passed mesh?
    resetSkeletonPose();
}

void SkeletonActor::resetSkeletonPose()
{
    if ( model && model->getMesh() )
        skeletonPose = SkeletonPose::createIdentityPoseInSkeletonSpace( *model->getMesh() );
    else
        skeletonPose.clear();
}
