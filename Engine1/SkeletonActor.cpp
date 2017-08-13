#include "SkeletonActor.h"

#include "SkeletonModel.h"
#include "SkeletonAnimation.h"

using namespace Engine1;

SkeletonActor::SkeletonActor( std::shared_ptr<SkeletonModel> model, const float43& pose ) :
    m_pose( pose ),
    m_model( model ),
    m_animationProgress( 0.0f ),
    m_animationSpeed( 0.0f )
{
    resetSkeletonPose();
}

SkeletonActor::SkeletonActor( std::shared_ptr<SkeletonModel> model, const float43& pose, const SkeletonPose& skeletonPose ) :
    m_pose( pose ),
    m_skeletonPose( skeletonPose ),
    m_model( model ),
    m_animationProgress( 0.0f ),
    m_animationSpeed( 0.0f )
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
    return m_pose;
}

float43& SkeletonActor::getPose( )
{
    return m_pose;
}

const SkeletonPose& SkeletonActor::getSkeletonPose( ) const
{
    return m_skeletonPose;
}

SkeletonPose& SkeletonActor::getSkeletonPose( )
{
    return m_skeletonPose;
}

std::shared_ptr<const SkeletonModel> SkeletonActor::getModel( ) const
{
    return m_model;
}

std::shared_ptr<SkeletonModel> SkeletonActor::getModel( )
{
    return m_model;
}

std::shared_ptr< const Model > SkeletonActor::getBaseModel() const
{
    return m_model;
}

std::shared_ptr< Model > SkeletonActor::getBaseModel()
{
    return m_model;
}

void SkeletonActor::setPose( const float43& pose )
{
    this->m_pose = pose;
}

void SkeletonActor::setSkeletonPose( const SkeletonPose& poseInSkeletonSpace )
{
    this->m_skeletonPose = poseInSkeletonSpace;
    //#TODO: should it check whether skeletonPose is correct for the passed mesh?
}

void SkeletonActor::setModel( std::shared_ptr<SkeletonModel> model )
{
    this->m_model = model;
    
    //#TODO: should it check whether skeletonPose is correct for the passed mesh?
    resetSkeletonPose();
}

void SkeletonActor::resetSkeletonPose()
{
    if ( m_model && m_model->getMesh() )
        m_skeletonPose = SkeletonPose::createIdentityPoseInSkeletonSpace( *m_model->getMesh() );
    else
        m_skeletonPose.clear();
}

void SkeletonActor::startAnimation( const std::shared_ptr< SkeletonAnimation > animationInSkeletonSpace )
{
    if ( !m_model || !m_model->getMesh() || !animationInSkeletonSpace || animationInSkeletonSpace->getKeyframeCount() == 0 ||
         animationInSkeletonSpace->getPose( 0 ).getBonesCount() != m_model->getMesh()->getBoneCount() )
        return;

    this->m_animation         = animationInSkeletonSpace;
    this->m_animationProgress = 0.0f;
    this->m_animationSpeed    = 1.0f / (float)animationInSkeletonSpace->getKeyframeCount(); // Temporarily assuming that whole animation takes 1 second.

    setSkeletonPose( animationInSkeletonSpace->getPose( 0 ) );
}

void SkeletonActor::updateAnimation( const float deltaTime )
{
    if (!m_animation)
        return;

    m_animationProgress += m_animationSpeed * deltaTime;
    m_animationProgress = fmod( m_animationProgress, 1.0f );

    setSkeletonPose( m_animation->getInterpolatedPose( m_animationProgress ) );
}
