#include "SkeletonActor.h"

#include "SkeletonModel.h"
#include "SkeletonAnimation.h"

using namespace Engine1;

SkeletonActor::SkeletonActor( std::shared_ptr<SkeletonModel> model, const float43& pose ) :
    pose( pose ),
    model( model ),
    animationProgress( 0.0f ),
    animationSpeed( 0.0f )
{
    resetSkeletonPose();
}

SkeletonActor::SkeletonActor( std::shared_ptr<SkeletonModel> model, const float43& pose, const SkeletonPose& skeletonPose ) :
    pose( pose ),
    skeletonPose( skeletonPose ),
    model( model ),
    animationProgress( 0.0f ),
    animationSpeed( 0.0f )
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

void SkeletonActor::startAnimation( const std::shared_ptr< SkeletonAnimation > animationInSkeletonSpace )
{
    if ( !model || !model->getMesh() || !animationInSkeletonSpace || animationInSkeletonSpace->getKeyframeCount() == 0 ||
         animationInSkeletonSpace->getPose( 0 ).getBonesCount() != model->getMesh()->getBoneCount() )
        return;

    this->animation         = animationInSkeletonSpace;
    this->animationProgress = 0.0f;
    this->animationSpeed    = 1.0f / (float)animationInSkeletonSpace->getKeyframeCount(); // Temporarily assuming that whole animation takes 1 second.

    setSkeletonPose( animationInSkeletonSpace->getPose( 0 ) );
}

void SkeletonActor::updateAnimation( const float deltaTime )
{
    if (!animation)
        return;

    animationProgress += animationSpeed * deltaTime;
    animationProgress = fmod( animationProgress, 1.0f );

    setSkeletonPose( animation->getInterpolatedPose( animationProgress ) );
}
